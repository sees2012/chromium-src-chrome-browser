// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_DRIVE_GDATA_WAPI_FEED_LOADER_OBSERVER_H_
#define CHROME_BROWSER_CHROMEOS_DRIVE_GDATA_WAPI_FEED_LOADER_OBSERVER_H_

#include <string>

class FilePath;

namespace gdata {

// Interface for classes that need to observe events from GdataWapiFeedLoader.
// All events are notified on UI thread.
class GDataWapiFeedLoaderObserver {
 public:
  // Triggered when a content of a directory has been changed.
  // |directory_path| is a virtual directory path representing the
  // changed directory.
  virtual void OnDirectoryChanged(const FilePath& directory_path) {
  }

  // Triggered when a document feed is fetched. |num_accumulated_entries|
  // tells the number of entries fetched so far.
  virtual void OnDocumentFeedFetched(int num_accumulated_entries) {
  }

  // Triggered when the feed from the server is loaded.
  virtual void OnFeedFromServerLoaded() {
  }

 protected:
  virtual ~GDataWapiFeedLoaderObserver() {}
};

}  // namespace gdata

#endif  // CHROME_BROWSER_CHROMEOS_DRIVE_GDATA_WAPI_FEED_LOADER_OBSERVER_H_