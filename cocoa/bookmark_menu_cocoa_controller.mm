// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "chrome/browser/cocoa/bookmark_menu_cocoa_controller.h"

#include "app/text_elider.h"
#include "base/sys_string_conversions.h"
#include "base/utf_string_conversions.h"
#include "chrome/app/chrome_dll_resource.h"  // IDC_BOOKMARK_MENU
#import "chrome/browser/app_controller_mac.h"
#include "chrome/browser/bookmarks/bookmark_model.h"
#include "chrome/browser/browser.h"
#import "chrome/browser/cocoa/bookmark_menu_bridge.h"
#include "chrome/browser/cocoa/event_utils.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/tab_contents/tab_contents.h"
#include "webkit/glue/window_open_disposition.h"

namespace {

// Menus more than this many pixels wide will get trimmed
// TODO(jrg): ask UI dudes what a good value is.
const NSUInteger kMaximumMenuPixelsWide = 300;

}

@implementation BookmarkMenuCocoaController

+ (NSString*)menuTitleForNode:(const BookmarkNode*)node {
  NSFont* nsfont = [NSFont menuBarFontOfSize:0];  // 0 means "default"
  gfx::Font font(base::SysNSStringToWide([nsfont fontName]),
                 static_cast<int>([nsfont pointSize]));
  std::wstring title = gfx::ElideText(UTF16ToWideHack(
                                          node->GetTitleAsString16()),
                                      font,
                                      kMaximumMenuPixelsWide,
                                      false);
  return base::SysWideToNSString(title);
}

- (id)initWithBridge:(BookmarkMenuBridge *)bridge {
  if ((self = [super init])) {
    bridge_ = bridge;
    DCHECK(bridge_);
    [[self menu] setDelegate:self];
  }
  return self;
}

- (void)dealloc {
  [[self menu] setDelegate:nil];
  [super dealloc];
}

- (NSMenu*)menu {
  return [[[NSApp mainMenu] itemWithTag:IDC_BOOKMARK_MENU] submenu];
}

- (BOOL)validateMenuItem:(NSMenuItem*)menuItem {
  AppController* controller = [NSApp delegate];
  return [controller keyWindowIsNotModal];
}

// NSMenu delegate method: called just before menu is displayed.
- (void)menuNeedsUpdate:(NSMenu*)menu {
  bridge_->UpdateMenu(menu);
}

// Return the a BookmarkNode that has the given id (called
// "identifier" here to avoid conflict with objc's concept of "id").
- (const BookmarkNode*)nodeForIdentifier:(int)identifier {
  return bridge_->GetBookmarkModel()->GetNodeByID(identifier);
}

// Open the URL of the given BookmarkNode in the current tab.
- (void)openURLForNode:(const BookmarkNode*)node {
  Browser* browser =
      Browser::GetOrCreateTabbedBrowser(bridge_->GetProfile());
  WindowOpenDisposition disposition =
      event_utils::WindowOpenDispositionFromNSEvent([NSApp currentEvent]);
  browser->OpenURL(node->GetURL(), GURL(), disposition,
                   PageTransition::AUTO_BOOKMARK);
}

- (IBAction)openBookmarkMenuItem:(id)sender {
  NSInteger tag = [sender tag];
  int identifier = tag;
  const BookmarkNode* node = [self nodeForIdentifier:identifier];
  DCHECK(node);
  if (!node)
    return;  // shouldn't be reached

  [self openURLForNode:node];
}

@end  // BookmarkMenuCocoaController

