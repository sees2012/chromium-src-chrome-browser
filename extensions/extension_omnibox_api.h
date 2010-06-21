// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_EXTENSIONS_EXTENSION_OMNIBOX_API_H_
#define CHROME_BROWSER_EXTENSIONS_EXTENSION_OMNIBOX_API_H_

#include "base/string16.h"
#include "chrome/browser/autocomplete/autocomplete.h"
#include "chrome/browser/extensions/extension_function.h"

// Event router class for events related to the omnibox API.
class ExtensionOmniboxEventRouter {
 public:
  // The user has changed what is typed into the omnibox while in an extension
  // keyword session. Returns true if someone is listening to this event, and
  // thus we have some degree of confidence we'll get a response.
  static bool OnInputChanged(
      Profile* profile, const std::string& extension_id,
      const std::string& input, int suggest_id);

  // The user has accepted the omnibox input.
  static void OnInputEntered(
      Profile* profile, const std::string& extension_id,
      const std::string& input);

 private:
  DISALLOW_COPY_AND_ASSIGN(ExtensionOmniboxEventRouter);
};

class OmniboxSendSuggestionsFunction : public SyncExtensionFunction {
 public:
  virtual bool RunImpl();
  DECLARE_EXTENSION_FUNCTION_NAME("experimental.omnibox.sendSuggestions");
};

struct ExtensionOmniboxSuggestion {
  // The text that gets put in the edit box.
  string16 content;

  // The text that is displayed in the drop down.
  string16 description;

  // Contains style ranges for the description.
  ACMatchClassifications description_styles;
};

struct ExtensionOmniboxSuggestions {
  int request_id;
  std::vector<ExtensionOmniboxSuggestion> suggestions;
};

#endif  // CHROME_BROWSER_EXTENSIONS_EXTENSION_OMNIBOX_API_H_
