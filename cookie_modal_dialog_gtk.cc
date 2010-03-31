// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/cookie_modal_dialog.h"

#include "app/gtk_util.h"
#include "app/l10n_util.h"
#include "base/logging.h"
#include "base/string_util.h"
#include "base/utf_string_conversions.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/gtk/gtk_chrome_cookie_view.h"
#include "chrome/browser/gtk/gtk_chrome_link_button.h"
#include "chrome/browser/gtk/gtk_util.h"
#include "chrome/browser/pref_service.h"
#include "chrome/browser/views/cookie_prompt_view.h"
#include "chrome/browser/tab_contents/tab_contents.h"
#include "chrome/common/pref_names.h"
#include "grit/generated_resources.h"
#include "grit/locale_settings.h"

namespace {

void OnExpanderActivate(GtkExpander* expander) {
  g_browser_process->local_state()->
      SetBoolean(prefs::kCookiePromptExpanded,
                 gtk_expander_get_expanded(GTK_EXPANDER(expander)));
}

}  // namespace

void CookiePromptModalDialog::CreateAndShowDialog() {
  dialog_ = CreateNativeDialog();
  gtk_widget_show_all(GTK_WIDGET(dialog_));

  // Suggest a minimum size.
  gint width;
  GtkRequisition req;
  gtk_widget_size_request(dialog_, &req);
  gtk_util::GetWidgetSizeFromResources(dialog_, IDS_ALERT_DIALOG_WIDTH_CHARS, 0,
                                       &width, NULL);
  if (width > req.width)
    gtk_widget_set_size_request(dialog_, width, -1);
}

void CookiePromptModalDialog::AcceptWindow() {
  HandleDialogResponse(GTK_DIALOG(dialog_), GTK_RESPONSE_ACCEPT);
}

void CookiePromptModalDialog::CancelWindow() {
  HandleDialogResponse(GTK_DIALOG(dialog_), GTK_RESPONSE_REJECT);
}

NativeDialog CookiePromptModalDialog::CreateNativeDialog() {
  gfx::NativeWindow window = tab_contents_->GetMessageBoxRootWindow();
  CookiePromptModalDialog::DialogType type = dialog_type();
  NativeDialog dialog = gtk_dialog_new_with_buttons(
      l10n_util::GetStringFUTF8(
          type == CookiePromptModalDialog::DIALOG_TYPE_COOKIE ?
          IDS_COOKIE_ALERT_TITLE : IDS_DATA_ALERT_TITLE,
          UTF8ToUTF16(origin().host())).c_str(),
      window,
      static_cast<GtkDialogFlags>(GTK_DIALOG_MODAL | GTK_DIALOG_NO_SEPARATOR),
      l10n_util::GetStringUTF8(IDS_COOKIE_ALERT_BLOCK_BUTTON).c_str(),
      GTK_RESPONSE_REJECT,
      l10n_util::GetStringUTF8(IDS_COOKIE_ALERT_ALLOW_BUTTON).c_str(),
      GTK_RESPONSE_ACCEPT,
      NULL);
  gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

  GtkWidget* content_area = GTK_DIALOG(dialog)->vbox;
  gtk_box_set_spacing(GTK_BOX(content_area), gtk_util::kContentAreaSpacing);

  string16 display_host = UTF8ToUTF16(origin().host());
  GtkWidget* label = gtk_util::LeftAlignMisc(gtk_label_new(
      l10n_util::GetStringFUTF8(
          type == CookiePromptModalDialog::DIALOG_TYPE_COOKIE ?
          IDS_COOKIE_ALERT_LABEL : IDS_DATA_ALERT_LABEL,
          display_host).c_str()));
  gtk_box_pack_start(GTK_BOX(content_area), label, FALSE, FALSE, 0);

  // Create a vbox for all the radio buttons so they aren't too far away from
  // each other.
  GtkWidget* radio_box = gtk_vbox_new(FALSE, gtk_util::kControlSpacing);
  remember_radio_ = gtk_radio_button_new_with_label(NULL,
      l10n_util::GetStringFUTF8(IDS_COOKIE_ALERT_REMEMBER_RADIO,
                                display_host).c_str());
  gtk_box_pack_start(GTK_BOX(radio_box), remember_radio_, FALSE, FALSE, 0);

  GtkWidget* ask_radio = gtk_radio_button_new_with_label_from_widget(
      GTK_RADIO_BUTTON(remember_radio_),
      l10n_util::GetStringUTF8(IDS_COOKIE_ALERT_ASK_RADIO).c_str());
  gtk_box_pack_start(GTK_BOX(radio_box), ask_radio, FALSE, FALSE, 0);

  gtk_box_pack_start(GTK_BOX(content_area), radio_box, FALSE, FALSE, 0);

  GtkWidget* expander = gtk_expander_new(
      l10n_util::GetStringUTF8(IDS_COOKIE_SHOW_DETAILS_LABEL).c_str());
  gtk_expander_set_expanded(GTK_EXPANDER(expander),
                            g_browser_process->local_state()->
                            GetBoolean(prefs::kCookiePromptExpanded));
  g_signal_connect(expander, "notify::expanded",
                   G_CALLBACK(OnExpanderActivate), NULL);

  GtkChromeCookieView* cookie_view = gtk_chrome_cookie_view_new();
  gtk_chrome_cookie_view_clear(cookie_view);
  if (type == CookiePromptModalDialog::DIALOG_TYPE_COOKIE) {
    gtk_chrome_cookie_view_display_cookie_string(cookie_view,
                                                 origin(), cookie_line());
  } else if (type == CookiePromptModalDialog::DIALOG_TYPE_LOCAL_STORAGE) {
    gtk_chrome_cookie_view_display_local_storage_item(
        cookie_view,
        origin().host(),
        local_storage_key(),
        local_storage_value());
  } else if (type == CookiePromptModalDialog::DIALOG_TYPE_DATABASE) {
    gtk_chrome_cookie_view_display_database_accessed(
        cookie_view,
        origin().host(),
        database_name());
  } else if (type == CookiePromptModalDialog::DIALOG_TYPE_APPCACHE) {
    gtk_chrome_cookie_view_display_appcache_created(
        cookie_view,
        appcache_manifest_url());
  } else {
    NOTIMPLEMENTED();
  }
  gtk_container_add(GTK_CONTAINER(expander), GTK_WIDGET(cookie_view));

  gtk_box_pack_end(GTK_BOX(content_area), GTK_WIDGET(expander),
                   FALSE, FALSE, 0);

  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);
  g_signal_connect(dialog, "response",
                   G_CALLBACK(AppModalDialog::OnDialogResponse),
                   reinterpret_cast<AppModalDialog*>(this));

  gtk_util::MakeAppModalWindowGroup();

  return dialog;
}

void CookiePromptModalDialog::HandleDialogResponse(GtkDialog* dialog,
                                                   gint response_id) {
  bool remember_radio = gtk_toggle_button_get_active(
      GTK_TOGGLE_BUTTON(remember_radio_));
  if (response_id == GTK_RESPONSE_REJECT) {
    BlockSiteData(remember_radio);
  } else if (response_id == GTK_RESPONSE_ACCEPT) {
    // TODO(erg): Needs to use |session_expire_| instead of true.
    AllowSiteData(remember_radio, true);
  } else {
    BlockSiteData(false);
  }
  gtk_widget_destroy(GTK_WIDGET(dialog));

  CompleteDialog();

  gtk_util::AppModalDismissedUngroupWindows();
  delete this;
}
