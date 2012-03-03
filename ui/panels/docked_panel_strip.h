// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_PANELS_DOCKED_PANEL_STRIP_H_
#define CHROME_BROWSER_UI_PANELS_DOCKED_PANEL_STRIP_H_
#pragma once

#include <list>
#include <set>
#include "base/basictypes.h"
#include "base/memory/weak_ptr.h"
#include "chrome/browser/ui/panels/auto_hiding_desktop_bar.h"
#include "chrome/browser/ui/panels/panel.h"
#include "chrome/browser/ui/panels/panel_strip.h"
#include "chrome/browser/ui/panels/panel_mouse_watcher_observer.h"
#include "ui/gfx/rect.h"

class Browser;
class PanelManager;

// This class manages a group of panels displayed in a horizontal strip,
// positioning the panels and controlling how they are displayed.
// Panels in the strip appear minimized, showing title-only or expanded.
// All panels in the strip are contained within the bounds of the strip.
class DockedPanelStrip : public PanelStrip,
                         public PanelMouseWatcherObserver {
 public:
  typedef std::list<Panel*> Panels;

  explicit DockedPanelStrip(PanelManager* panel_manager);
  virtual ~DockedPanelStrip();

  // PanelStrip OVERRIDES:
  virtual void SetDisplayArea(const gfx::Rect& display_area) OVERRIDE;

  // Rearranges the positions of the panels in the strip.
  // Handles moving panels to/from overflow area as needed.
  // This is called when the display space has been changed, i.e. working
  // area being changed or a panel being closed.
  virtual void RefreshLayout() OVERRIDE;

  // Adds a panel to the strip. The panel may be a newly created panel or one
  // that is transitioning from another grouping of panels.
  virtual void AddPanel(Panel* panel) OVERRIDE;
  virtual void RemovePanel(Panel* panel) OVERRIDE;
  virtual void CloseAll() OVERRIDE;
  virtual void ResizePanelWindow(
      Panel* panel,
      const gfx::Size& preferred_window_size) OVERRIDE;
  virtual void OnPanelAttentionStateChanged(Panel* panel) OVERRIDE;
  virtual void ActivatePanel(Panel* panel) OVERRIDE;
  virtual void MinimizePanel(Panel* panel) OVERRIDE;
  virtual void RestorePanel(Panel* panel) OVERRIDE;
  virtual bool IsPanelMinimized(const Panel* panel) const OVERRIDE;
  virtual bool CanShowPanelAsActive(const Panel* panel) const OVERRIDE;
  virtual bool CanDragPanel(const Panel* panel) const OVERRIDE;
  virtual void StartDraggingPanel(Panel* panel) OVERRIDE;
  virtual void DragPanel(Panel* panel, int delta_x, int delta_y) OVERRIDE;
  virtual void EndDraggingPanel(Panel* panel, bool cancelled) OVERRIDE;

  // Invoked when a panel's expansion state changes.
  void OnPanelExpansionStateChanged(Panel* panel);

  // Returns true if we should bring up the titlebars, given the current mouse
  // point.
  bool ShouldBringUpTitlebars(int mouse_x, int mouse_y) const;

  // Brings up or down the titlebars for all minimized panels.
  void BringUpOrDownTitlebars(bool bring_up);

  // Returns the bottom position for the panel per its expansion state. If auto-
  // hide bottom bar is present, we want to move the minimized panel to the
  // bottom of the screen, not the bottom of the work area.
  int GetBottomPositionForExpansionState(
      Panel::ExpansionState expansion_state) const;

  // num_panels() and panels() only includes panels in the panel strip that
  // do NOT have a temporary layout.
  int num_panels() const { return panels_.size(); }
  const Panels& panels() const { return panels_; }
  Panel* last_panel() const { return panels_.empty() ? NULL : panels_.back(); }

  gfx::Rect display_area() const { return display_area_; }

  int GetMaxPanelWidth() const;
  int GetMaxPanelHeight() const;
  int StartingRightPosition() const;

  void OnAutoHidingDesktopBarVisibilityChanged(
      AutoHidingDesktopBar::Alignment alignment,
      AutoHidingDesktopBar::Visibility visibility);

  void OnFullScreenModeChanged(bool is_full_screen);

  // Returns |true| if panel can fit in the dock strip.
  bool CanFitPanel(const Panel* panel) const;

#ifdef UNIT_TEST
  int num_temporary_layout_panels() const {
    return panels_in_temporary_layout_.size();
  }
#endif

 private:
  enum TitlebarAction {
    NO_ACTION,
    BRING_UP,
    BRING_DOWN
  };

  // Overridden from PanelMouseWatcherObserver:
  virtual void OnMouseMove(const gfx::Point& mouse_position) OVERRIDE;

  // Keep track of the minimized panels to control mouse watching.
  void IncrementMinimizedPanels();
  void DecrementMinimizedPanels();

  // Help functions to drag the given panel.
  void DragLeft(Panel* dragging_panel);
  void DragRight(Panel* dragging_panel);

  // Does the real job of bringing up or down the titlebars.
  void DoBringUpOrDownTitlebars(bool bring_up);
  // The callback for a delyed task, checks if it still need to perform
  // the delayed action.
  void DelayedBringUpOrDownTitlebarsCheck();

  int GetRightMostAvailablePosition() const;

  // Determines position in strip where a panel of |width| will fit.
  // Other panels in the strip may be moved to overflow to make room.
  // Returns x position where a panel of |width| wide can fit.
  // |width| is in screen coordinates.
  int FitPanelWithWidth(int width);

  // Called by AddPanel() after a delay to move a newly created panel from
  // the panel strip to overflow because the panel could not fit
  // within the bounds of the panel strip. New panels are first displayed
  // in the panel strip, then moved to overflow so that all created
  // panels are (at least briefly) visible before entering overflow.
  void DelayedMovePanelToOverflow(Panel* panel);

  Panel* dragging_panel() const;

  PanelManager* panel_manager_;  // Weak, owns us.

  // All panels in the panel strip must fit within this area.
  gfx::Rect display_area_;

  Panels panels_;

  // Stores newly created panels that have a temporary layout until they
  // are moved to overflow after a delay.
  std::set<Panel*> panels_in_temporary_layout_;

  int minimized_panel_count_;
  bool are_titlebars_up_;

  // Referring to current position in |panels_| where the dragging panel
  // resides.
  Panels::iterator dragging_panel_current_iterator_;

  // Referring to original position in |panels_| where the dragging panel
  // resides.
  Panels::iterator dragging_panel_original_iterator_;

  // Delayed transitions support. Sometimes transitions between minimized and
  // title-only states are delayed, for better usability with Taskbars/Docks.
  TitlebarAction delayed_titlebar_action_;

  // Owned by MessageLoop after posting.
  base::WeakPtrFactory<DockedPanelStrip> titlebar_action_factory_;

  static const int kPanelsHorizontalSpacing = 4;

  // Absolute minimum width and height for panels, including non-client area.
  // Should only be big enough to accomodate a close button on the reasonably
  // recognisable titlebar.
  static const int kPanelMinWidth;
  static const int kPanelMinHeight;

  DISALLOW_COPY_AND_ASSIGN(DockedPanelStrip);
};

#endif  // CHROME_BROWSER_UI_PANELS_DOCKED_PANEL_STRIP_H_
