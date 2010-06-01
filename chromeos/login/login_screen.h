// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_LOGIN_LOGIN_SCREEN_H_
#define CHROME_BROWSER_CHROMEOS_LOGIN_LOGIN_SCREEN_H_

#include <string>

#include "base/ref_counted.h"
#include "chrome/browser/chromeos/login/authenticator.h"
#include "chrome/browser/chromeos/login/login_status_consumer.h"
#include "chrome/browser/chromeos/login/new_user_view.h"
#include "chrome/browser/chromeos/login/view_screen.h"
#include "chrome/browser/views/info_bubble.h"

namespace chromeos {

class MessageBubble;

class LoginScreen : public ViewScreen<NewUserView>,
                    public NewUserView::Delegate,
                    public LoginStatusConsumer,
                    public InfoBubbleDelegate {
 public:
  explicit LoginScreen(WizardScreenDelegate* delegate);
  virtual ~LoginScreen();

  bool IsErrorShown() {
    return bubble_ != NULL;
  }

  // NewUserView::Delegate:
  virtual void OnLogin(const std::string& username,
                       const std::string& password);
  virtual void OnLoginOffTheRecord();
  virtual void OnCreateAccount();
  virtual void ClearErrors();

  // Overridden from LoginStatusConsumer.
  virtual void OnLoginFailure(const std::string& error);
  virtual void OnLoginSuccess(const std::string& username,
                              const std::string& credentials);
  virtual void OnOffTheRecordLoginSuccess();

  // Overridden from views::InfoBubbleDelegate.
  virtual void InfoBubbleClosing(InfoBubble* info_bubble,
                                 bool closed_by_escape) {
    bubble_ = NULL;
  }
  virtual bool CloseOnEscape() { return true; }
  virtual bool FadeInOnShow() { return false; }

 private:
  // ViewScreen<NewUserView>:
  virtual NewUserView* AllocateView();

  // Shows error message with the specified message id.
  // If |details| string is not empty, it specify additional error text
  // provided by authenticator, it is not localized.
  void ShowError(int error_id, const std::string& details);

  // Pointer to shown message bubble. We don't need to delete it because
  // it will be deleted on bubble closing.
  MessageBubble* bubble_;

  scoped_refptr<Authenticator> authenticator_;

  DISALLOW_COPY_AND_ASSIGN(LoginScreen);
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_LOGIN_LOGIN_SCREEN_H_
