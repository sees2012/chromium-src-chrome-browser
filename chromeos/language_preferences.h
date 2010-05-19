// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_LANGUAGE_PREFERENCES_H_
#define CHROME_BROWSER_CHROMEOS_LANGUAGE_PREFERENCES_H_

#include "base/basictypes.h"
#include "chrome/common/pref_names.h"
#include "grit/generated_resources.h"

// Section and config names for the IBus configuration daemon.
namespace chromeos {

// For ibus-daemon
const char kGeneralSectionName[] = "general";
const char kHotKeySectionName[] = "general/hotkey";
const char kPreloadEnginesConfigName[] = "preload_engines";
const char kNextEngineInMenuConfigName[] = "next_engine_in_menu";
const char kPreviousEngineConfigName[] = "previous_engine";

// TODO(suzhe): In order to avoid blocking any accelerator keys with alt+shift
// modifiers, we should use release events of alt+shift key binding instead of
// press events. The corresponding release events are:
// "Alt+Shift+Shift_L+Release" and "Alt+Shift+Meta_L+Release"
// But unfortunately http://crbug.com/40754 prevents these release events
// from taking effect.
// TODO(yusukes): Check if the "Kana/Eisu" key in the Japanese keyboard for
// Chrome OS actually generates Zenkaku_Hankaku when the keyboard gets ready.

// ibus-daemon accepts up to 5 next-engine hot-keys.
const wchar_t kHotkeyNextEngineInMenu[] =
    L"Alt+Shift_L,Shift+Meta_L,Control+Shift+space,Zenkaku_Hankaku";
// TODO(suzhe): Add more key bindings?
const wchar_t kHotkeyPreviousEngine[] = L"Control+space";

// For Korean input method (ibus-hangul)
const char kHangulSectionName[] = "engine/Hangul";
const char kHangulKeyboardConfigName[] = "HangulKeyboard";

const struct HangulKeyboardNameIDPair {
  int message_id;
  const wchar_t* keyboard_id;
} kHangulKeyboardNameIDPairs[] = {
  // We have to sync the |keyboard_id|s with those in
  // ibus-hangul/files/setup/main.py.
  { IDS_OPTIONS_SETTINGS_LANGUAGES_HANGUL_SETTINGS_KEYBOARD_2_SET, L"2" },
  { IDS_OPTIONS_SETTINGS_LANGUAGES_HANGUL_SETTINGS_KEYBOARD_3_SET_FINAL,
    L"3f" },
  { IDS_OPTIONS_SETTINGS_LANGUAGES_HANGUL_SETTINGS_KEYBOARD_3_SET_390, L"39" },
  { IDS_OPTIONS_SETTINGS_LANGUAGES_HANGUL_SETTINGS_KEYBOARD_3_SET_NO_SHIFT,
    L"3s" },
  // We don't support "Sebeolsik 2 set" keyboard.
};

// For Simplified Chinese input method (ibus-pinyin)
const char kPinyinSectionName[] = "engine/Pinyin";

// We have to sync the |ibus_config_name|s with those in
// ibus-pinyin/files/src/Config.cc.
const struct {
  const wchar_t* pref_name;
  const char* ibus_config_name;
  bool default_value;
  int message_id;
} kPinyinBooleanPrefs[] = {
  { prefs::kLanguagePinyinCorrectPinyin, "CorrectPinyin", true,
    IDS_OPTIONS_SETTINGS_LANGUAGES_PINYIN_SETTING_CORRECT_PINYIN },
  { prefs::kLanguagePinyinFuzzyPinyin, "FuzzyPinyin", false,
    IDS_OPTIONS_SETTINGS_LANGUAGES_PINYIN_SETTING_FUZZY_PINYIN },
  { prefs::kLanguagePinyinShiftSelectCandidate, "ShiftSelectCandidate",
    false, IDS_OPTIONS_SETTINGS_LANGUAGES_PINYIN_SETTING_SHIFT_SELECT_PINYIN },
  { prefs::kLanguagePinyinMinusEqualPage, "MinusEqualPage", true,
    IDS_OPTIONS_SETTINGS_LANGUAGES_PINYIN_SETTING_MINUS_EQUAL_PAGE },
  { prefs::kLanguagePinyinCommaPeriodPage, "CommaPeriodPage", true,
    IDS_OPTIONS_SETTINGS_LANGUAGES_PINYIN_SETTING_COMMA_PERIOD_PAGE },
  { prefs::kLanguagePinyinAutoCommit, "AutoCommit", false,
    IDS_OPTIONS_SETTINGS_LANGUAGES_PINYIN_SETTING_AUTO_COMMIT },
  { prefs::kLanguagePinyinDoublePinyin, "DoublePinyin", false,
    IDS_OPTIONS_SETTINGS_LANGUAGES_PINYIN_SETTING_DOUBLE_PINYIN },
  { prefs::kLanguagePinyinInitChinese, "InitChinese", true,
    IDS_OPTIONS_SETTINGS_LANGUAGES_PINYIN_SETTING_INIT_CHINESE },
  { prefs::kLanguagePinyinInitFull, "InitFull", false,
    IDS_OPTIONS_SETTINGS_LANGUAGES_PINYIN_SETTING_INIT_FULL },
  { prefs::kLanguagePinyinInitFullPunct, "InitFullPunct", true,
    IDS_OPTIONS_SETTINGS_LANGUAGES_PINYIN_SETTING_INIT_FULL_PUNCT },
  { prefs::kLanguagePinyinInitSimplifiedChinese, "InitSimplifiedChinese",
    true,
    IDS_OPTIONS_SETTINGS_LANGUAGES_PINYIN_SETTING_INIT_SIMPLIFIED_CHINESE },
  { prefs::kLanguagePinyinTradCandidate, "TradCandidate", false,
    IDS_OPTIONS_SETTINGS_LANGUAGES_PINYIN_SETTING_TRAD_CANDIDATE },
  // TODO(yusukes): Support PINYIN_{INCOMPLETE,CORRECT,FUZZY}_... prefs (32
  // additional boolean prefs.)
};
const size_t kNumPinyinBooleanPrefs = ARRAYSIZE_UNSAFE(kPinyinBooleanPrefs);
// TODO(yusukes): Support HalfWidthPuncts and IncompletePinyin prefs if needed.

const struct {
  const wchar_t* pref_name;
  const char* ibus_config_name;
  int default_value;
  // TODO(yusukes): Add message_id if needed.
} kPinyinIntegerPrefs[] = {
  { prefs::kLanguagePinyinDoublePinyinSchema, "DoublePinyinSchema", 0 },
  // TODO(yusukes): the type of lookup_table_page_size on ibus should be uint.
  { prefs::kLanguagePinyinLookupTablePageSize, "LookupTablePageSize", 5 },
};
const size_t kNumPinyinIntegerPrefs = ARRAYSIZE_UNSAFE(kPinyinIntegerPrefs);

// For Traditional Chinese input method (ibus-chewing)

// For Japanese input method (ibus-mozc)

// TODO(yusukes): Add constants for these components.
}  // chromeos

#endif  // CHROME_BROWSER_CHROMEOS_LANGUAGE_PREFERENCES_H_
