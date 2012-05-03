// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_MEDIA_MEDIA_STREAM_CAPTURE_INDICATOR_H_
#define CHROME_BROWSER_MEDIA_MEDIA_STREAM_CAPTURE_INDICATOR_H_
#pragma once

#include <list>
#include <string>

#include "base/memory/ref_counted.h"
#include "content/public/common/media_stream_request.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "ui/base/models/simple_menu_model.h"

class StatusIcon;
class StatusTray;

// This indicator is owned by MediaInternals and deleted when MediaInternals
// is deleted.
class MediaStreamCaptureIndicator
    : public base::RefCountedThreadSafe<MediaStreamCaptureIndicator>,
      public ui::SimpleMenuModel::Delegate {
 public:
  MediaStreamCaptureIndicator();

  // Overrides from SimpleMenuModel::Delegate implementation.
  virtual bool IsCommandIdChecked(int command_id) const OVERRIDE;
  virtual bool IsCommandIdEnabled(int command_id) const OVERRIDE;
  virtual bool GetAcceleratorForCommandId(
      int command_id,
      ui::Accelerator* accelerator) OVERRIDE;
  virtual void ExecuteCommand(int command_id) OVERRIDE;

  // Called on IO thread when MediaStream opens new capture devices.
  void CaptureDevicesOpened(int render_process_id,
                            int render_view_id,
                            const content::MediaStreamDevices& devices);

  // Called on IO thread when MediaStream closes the opened devices.
  void CaptureDevicesClosed(int render_process_id,
                            int render_view_id,
                            const content::MediaStreamDevices& devices);

 private:
  // Struct to store the usage information of the capture devices for each tab.
  struct CaptureDeviceTab {
    CaptureDeviceTab(int render_process_id,
                     int render_view_id,
                     content::MediaStreamDeviceType type)
        : render_process_id(render_process_id),
          render_view_id(render_view_id),
          type(type) {}

    int render_process_id;
    int render_view_id;
    content::MediaStreamDeviceType type;
  };

  // A private predicate used in std::find_if to find a |CaptureDeviceTab|
  // which matches the information specified at construction.
  class TabEquals {
   public:
    TabEquals(int render_process_id, int render_view_id,
              content::MediaStreamDeviceType type);
    TabEquals(int render_process_id, int render_view_id);

    bool operator() (
        const MediaStreamCaptureIndicator::CaptureDeviceTab& tab);

   private:
     int render_process_id_;
     int render_view_id_;
     content::MediaStreamDeviceType type_;
  };

  friend class base::RefCountedThreadSafe<MediaStreamCaptureIndicator>;
  virtual ~MediaStreamCaptureIndicator();

  // Called by the public functions, executed on UI thread.
  void DoDevicesOpenedOnUIThread(int render_process_id,
                                 int render_view_id,
                                 const content::MediaStreamDevices& devices);
  void DoDevicesClosedOnUIThread(int render_process_id,
                                 int render_view_id,
                                 const content::MediaStreamDevices& devices);

  // Following functions/variables are executed/accessed only on UI thread.
  // Creates the status tray if it has not been created.
  void CreateStatusTray();

  // Makes sure we have done one-time initialization of the |icon_image_|.
  void EnsureStatusTrayIcon();

  // Triggers a balloon in the corner telling capture devices are being used.
  void ShowBalloon(int render_process_id, int render_view_id,
                   const content::MediaStreamDevices& devices) const;

  // Hides the status tray from the desktop.
  void Hide();

  // Adds the new tab to the device usage list.
  void AddCaptureDeviceTab(int render_process_id,
                           int render_view_id,
                           const content::MediaStreamDevices& devices);

  // Removes the tab from the device usage list.
  void RemoveCaptureDeviceTab(int render_process_id,
                              int render_view_id,
                              const content::MediaStreamDevices& devices);

  // Gets the title of the tab.
  string16 GetTitle(int render_process_id, int render_view_id) const;

  // Updates the status tray menu with the new device list. This call will be
  // triggered by both AddCaptureDeviceTab() and RemoveCaptureDeviceTab().
  void UpdateStatusTrayIconContextMenu();

  // Reference to our status icon - owned by the StatusTray. If null,
  // the platform doesn't support status icons.
  StatusIcon* status_icon_;

  // Icon to be displayed on the status tray.
  SkBitmap icon_image_;

  // A list that contains the usage information of the opened capture devices.
  typedef std::list<CaptureDeviceTab> CaptureDeviceTabList;
  CaptureDeviceTabList tabs_;
};

#endif  // CHROME_BROWSER_MEDIA_MEDIA_STREAM_CAPTURE_INDICATOR_H_
