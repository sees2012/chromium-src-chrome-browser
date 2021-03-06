// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/extension_function_registry.h"

#include "chrome/browser/accessibility/accessibility_extension_api.h"
#include "chrome/browser/extensions/api/app/app_api.h"
#include "chrome/browser/extensions/api/bookmark_manager_private/bookmark_manager_private_api.h"
#include "chrome/browser/extensions/api/browsing_data/browsing_data_api.h"
#include "chrome/browser/extensions/api/commands/commands.h"
#include "chrome/browser/extensions/api/extension_action/extension_browser_actions_api.h"
#include "chrome/browser/extensions/api/extension_action/extension_page_actions_api.h"
#include "chrome/browser/extensions/api/extension_action/extension_script_badge_api.h"
#include "chrome/browser/extensions/api/idle/idle_api.h"
#include "chrome/browser/extensions/api/managed_mode/managed_mode_api.h"
#include "chrome/browser/extensions/api/metrics/metrics.h"
#include "chrome/browser/extensions/api/module/module.h"
#include "chrome/browser/extensions/api/omnibox/omnibox_api.h"
#include "chrome/browser/extensions/api/preference/preference_api.h"
#include "chrome/browser/extensions/api/runtime/runtime_api.h"
#include "chrome/browser/extensions/api/test/test_api.h"
#include "chrome/browser/extensions/api/top_sites/top_sites_api.h"
#include "chrome/browser/extensions/api/web_request/web_request_api.h"
#include "chrome/browser/extensions/api/web_socket_proxy_private/web_socket_proxy_private_api.h"
#include "chrome/browser/extensions/api/webstore_private/webstore_private_api.h"
#include "chrome/browser/extensions/system/system_api.h"
#include "chrome/browser/infobars/infobar_extension_api.h"
#include "chrome/browser/rlz/rlz_extension_api.h"
#include "chrome/browser/speech/speech_input_extension_api.h"
#include "chrome/common/extensions/api/generated_api.h"

#if defined(TOOLKIT_VIEWS)
#include "chrome/browser/extensions/api/input/input.h"
#endif  // defined(TOOLKIT_VIEWS)

#if defined(OS_CHROMEOS)
#include "chrome/browser/chromeos/extensions/echo_private_api.h"
#include "chrome/browser/chromeos/extensions/info_private_api.h"
#include "chrome/browser/chromeos/extensions/input_method_api.h"
#include "chrome/browser/chromeos/extensions/media_player_api.h"
#include "chrome/browser/chromeos/extensions/power/power_api.h"
#include "chrome/browser/chromeos/extensions/wallpaper_private_api.h"
#include "chrome/browser/extensions/api/terminal/terminal_private_api.h"
#endif  // defined(OS_CHROMEOS)

// static
ExtensionFunctionRegistry* ExtensionFunctionRegistry::GetInstance() {
  return Singleton<ExtensionFunctionRegistry>::get();
}

ExtensionFunctionRegistry::ExtensionFunctionRegistry() {
  ResetFunctions();
}

ExtensionFunctionRegistry::~ExtensionFunctionRegistry() {
}

void ExtensionFunctionRegistry::ResetFunctions() {
#if defined(ENABLE_EXTENSIONS)

  // Register all functions here.

  // Page Actions.
  RegisterFunction<EnablePageActionsFunction>();
  RegisterFunction<DisablePageActionsFunction>();
  RegisterFunction<PageActionShowFunction>();
  RegisterFunction<PageActionHideFunction>();
  RegisterFunction<PageActionSetIconFunction>();
  RegisterFunction<PageActionSetTitleFunction>();
  RegisterFunction<PageActionSetPopupFunction>();
  RegisterFunction<PageActionGetTitleFunction>();
  RegisterFunction<PageActionGetPopupFunction>();

  // Browser Actions.
  RegisterFunction<BrowserActionSetIconFunction>();
  RegisterFunction<BrowserActionSetTitleFunction>();
  RegisterFunction<BrowserActionSetBadgeTextFunction>();
  RegisterFunction<BrowserActionSetBadgeBackgroundColorFunction>();
  RegisterFunction<BrowserActionSetPopupFunction>();
  RegisterFunction<BrowserActionGetTitleFunction>();
  RegisterFunction<BrowserActionGetBadgeTextFunction>();
  RegisterFunction<BrowserActionGetBadgeBackgroundColorFunction>();
  RegisterFunction<BrowserActionGetPopupFunction>();
  RegisterFunction<BrowserActionEnableFunction>();
  RegisterFunction<BrowserActionDisableFunction>();

  // Script Badges.
  RegisterFunction<ScriptBadgeGetAttentionFunction>();
  RegisterFunction<ScriptBadgeGetPopupFunction>();
  RegisterFunction<ScriptBadgeSetPopupFunction>();

  // Browsing Data.
  RegisterFunction<RemoveBrowsingDataFunction>();
  RegisterFunction<RemoveAppCacheFunction>();
  RegisterFunction<RemoveCacheFunction>();
  RegisterFunction<RemoveCookiesFunction>();
  RegisterFunction<RemoveDownloadsFunction>();
  RegisterFunction<RemoveFileSystemsFunction>();
  RegisterFunction<RemoveFormDataFunction>();
  RegisterFunction<RemoveHistoryFunction>();
  RegisterFunction<RemoveIndexedDBFunction>();
  RegisterFunction<RemoveLocalStorageFunction>();
  RegisterFunction<RemovePluginDataFunction>();
  RegisterFunction<RemovePasswordsFunction>();
  RegisterFunction<RemoveWebSQLFunction>();

  // Infobars.
  RegisterFunction<ShowInfoBarFunction>();

  // BookmarkManager
  RegisterFunction<extensions::CopyBookmarkManagerFunction>();
  RegisterFunction<extensions::CutBookmarkManagerFunction>();
  RegisterFunction<extensions::PasteBookmarkManagerFunction>();
  RegisterFunction<extensions::CanPasteBookmarkManagerFunction>();
  RegisterFunction<extensions::SortChildrenBookmarkManagerFunction>();
  RegisterFunction<extensions::BookmarkManagerGetStringsFunction>();
  RegisterFunction<extensions::StartDragBookmarkManagerFunction>();
  RegisterFunction<extensions::DropBookmarkManagerFunction>();
  RegisterFunction<extensions::GetSubtreeBookmarkManagerFunction>();
  RegisterFunction<extensions::CanEditBookmarkManagerFunction>();
  RegisterFunction<extensions::CanOpenNewWindowsBookmarkFunction>();

  // Idle
  RegisterFunction<extensions::IdleQueryStateFunction>();
  RegisterFunction<extensions::IdleSetDetectionIntervalFunction>();

  // Metrics.
  RegisterFunction<extensions::MetricsRecordUserActionFunction>();
  RegisterFunction<extensions::MetricsRecordValueFunction>();
  RegisterFunction<extensions::MetricsRecordPercentageFunction>();
  RegisterFunction<extensions::MetricsRecordCountFunction>();
  RegisterFunction<extensions::MetricsRecordSmallCountFunction>();
  RegisterFunction<extensions::MetricsRecordMediumCountFunction>();
  RegisterFunction<extensions::MetricsRecordTimeFunction>();
  RegisterFunction<extensions::MetricsRecordMediumTimeFunction>();
  RegisterFunction<extensions::MetricsRecordLongTimeFunction>();

  // RLZ (not supported on ChromeOS yet).
#if defined(ENABLE_RLZ) && !defined(OS_CHROMEOS)
  RegisterFunction<RlzRecordProductEventFunction>();
  RegisterFunction<RlzGetAccessPointRlzFunction>();
  RegisterFunction<RlzSendFinancialPingFunction>();
  RegisterFunction<RlzClearProductStateFunction>();
#endif

  // Test.
  RegisterFunction<extensions::TestNotifyPassFunction>();
  RegisterFunction<extensions::TestFailFunction>();
  RegisterFunction<extensions::TestLogFunction>();
  RegisterFunction<extensions::TestResetQuotaFunction>();
  RegisterFunction<extensions::TestCreateIncognitoTabFunction>();
  RegisterFunction<extensions::TestSendMessageFunction>();
  RegisterFunction<extensions::TestGetConfigFunction>();

  // Accessibility.
  RegisterFunction<GetFocusedControlFunction>();
  RegisterFunction<SetAccessibilityEnabledFunction>();
  RegisterFunction<GetAlertsForTabFunction>();

  // Commands.
  RegisterFunction<GetAllCommandsFunction>();

  // Omnibox.
  RegisterFunction<extensions::OmniboxSendSuggestionsFunction>();
  RegisterFunction<extensions::OmniboxSetDefaultSuggestionFunction>();

#if defined(ENABLE_INPUT_SPEECH)
  // Speech input.
  RegisterFunction<StartSpeechInputFunction>();
  RegisterFunction<StopSpeechInputFunction>();
  RegisterFunction<IsRecordingSpeechInputFunction>();
#endif

#if defined(TOOLKIT_VIEWS)
  // Input.
  RegisterFunction<extensions::SendKeyboardEventInputFunction>();
#endif

#if defined(OS_CHROMEOS)
  // Power
  RegisterFunction<extensions::power::RequestKeepAwakeFunction>();
  RegisterFunction<extensions::power::ReleaseKeepAwakeFunction>();
#endif

  // Managed mode.
  RegisterFunction<extensions::GetManagedModeFunction>();
  RegisterFunction<extensions::EnterManagedModeFunction>();
  RegisterFunction<extensions::GetPolicyFunction>();
  RegisterFunction<extensions::SetPolicyFunction>();

  // Extension module.
  RegisterFunction<extensions::SetUpdateUrlDataFunction>();
  RegisterFunction<extensions::IsAllowedIncognitoAccessFunction>();
  RegisterFunction<extensions::IsAllowedFileSchemeAccessFunction>();

  // WebstorePrivate.
  RegisterFunction<extensions::GetBrowserLoginFunction>();
  RegisterFunction<extensions::GetStoreLoginFunction>();
  RegisterFunction<extensions::SetStoreLoginFunction>();
  RegisterFunction<extensions::InstallBundleFunction>();
  RegisterFunction<extensions::BeginInstallWithManifestFunction>();
  RegisterFunction<extensions::CompleteInstallFunction>();
  RegisterFunction<extensions::GetWebGLStatusFunction>();

  // WebRequest.
  RegisterFunction<WebRequestAddEventListener>();
  RegisterFunction<WebRequestEventHandled>();

  // Preferences.
  RegisterFunction<extensions::GetPreferenceFunction>();
  RegisterFunction<extensions::SetPreferenceFunction>();
  RegisterFunction<extensions::ClearPreferenceFunction>();

  // ChromeOS-specific part of the API.
#if defined(OS_CHROMEOS)
  // Device Customization.
  RegisterFunction<extensions::GetChromeosInfoFunction>();

  // Mediaplayer
  RegisterFunction<extensions::PlayMediaplayerFunction>();
  RegisterFunction<extensions::GetPlaylistMediaplayerFunction>();
  RegisterFunction<extensions::SetWindowHeightMediaplayerFunction>();
  RegisterFunction<extensions::CloseWindowMediaplayerFunction>();

  // WallpaperManagerPrivate functions.
  RegisterFunction<WallpaperStringsFunction>();
  RegisterFunction<WallpaperSetWallpaperIfExistFunction>();
  RegisterFunction<WallpaperSetWallpaperFunction>();
  RegisterFunction<WallpaperSetCustomWallpaperFunction>();
  RegisterFunction<WallpaperMinimizeInactiveWindowsFunction>();
  RegisterFunction<WallpaperRestoreMinimizedWindowsFunction>();
  RegisterFunction<WallpaperGetThumbnailFunction>();
  RegisterFunction<WallpaperSaveThumbnailFunction>();
  RegisterFunction<WallpaperGetOfflineWallpaperListFunction>();

  // InputMethod
  RegisterFunction<extensions::GetInputMethodFunction>();

  // Echo
  RegisterFunction<GetRegistrationCodeFunction>();

  // Terminal
  RegisterFunction<OpenTerminalProcessFunction>();
  RegisterFunction<SendInputToTerminalProcessFunction>();
  RegisterFunction<CloseTerminalProcessFunction>();
  RegisterFunction<OnTerminalResizeFunction>();
#endif

  // Websocket to TCP proxy. Currently noop on anything other than ChromeOS.
  RegisterFunction<
      extensions::WebSocketProxyPrivateGetPassportForTCPFunction>();
  RegisterFunction<extensions::WebSocketProxyPrivateGetURLForTCPFunction>();

  // Experimental App API.
  RegisterFunction<extensions::AppNotifyFunction>();
  RegisterFunction<extensions::AppClearAllNotificationsFunction>();

  // TopSites
  RegisterFunction<extensions::GetTopSitesFunction>();

  // System
  RegisterFunction<extensions::GetIncognitoModeAvailabilityFunction>();
  RegisterFunction<extensions::GetUpdateStatusFunction>();

  // Runtime
  RegisterFunction<extensions::RuntimeGetBackgroundPageFunction>();
  RegisterFunction<extensions::RuntimeReloadFunction>();
  RegisterFunction<extensions::RuntimeRequestUpdateCheckFunction>();

  // Generated APIs
  extensions::api::GeneratedFunctionRegistry::RegisterAll(this);
#endif  // defined(ENABLE_EXTENSIONS)
}

void ExtensionFunctionRegistry::GetAllNames(std::vector<std::string>* names) {
  for (FactoryMap::iterator iter = factories_.begin();
       iter != factories_.end(); ++iter) {
    names->push_back(iter->first);
  }
}

bool ExtensionFunctionRegistry::OverrideFunction(
    const std::string& name,
    ExtensionFunctionFactory factory) {
  FactoryMap::iterator iter = factories_.find(name);
  if (iter == factories_.end()) {
    return false;
  } else {
    iter->second = factory;
    return true;
  }
}

ExtensionFunction* ExtensionFunctionRegistry::NewFunction(
    const std::string& name) {
  FactoryMap::iterator iter = factories_.find(name);
  DCHECK(iter != factories_.end());
  ExtensionFunction* function = iter->second();
  function->set_name(name);
  return function;
}
