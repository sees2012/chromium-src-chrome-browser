// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/google/google_url_tracker.h"

#include <vector>

#include "app/l10n_util.h"
#include "base/command_line.h"
#include "base/compiler_specific.h"
#include "base/string_util.h"
#include "base/utf_string_conversions.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/prefs/pref_service.h"
#include "chrome/browser/profile.h"
#include "chrome/browser/search_engines/template_url.h"
#include "chrome/browser/tab_contents/infobar_delegate.h"
#include "chrome/browser/tab_contents/navigation_controller.h"
#include "chrome/browser/tab_contents/tab_contents.h"
#include "chrome/common/chrome_switches.h"
#include "chrome/common/net/url_fetcher_protect.h"
#include "chrome/common/notification_service.h"
#include "chrome/common/pref_names.h"
#include "grit/generated_resources.h"
#include "net/base/load_flags.h"
#include "net/url_request/url_request_status.h"

// GoogleURLTrackerInfoBarDelegate --------------------------------------------

class GoogleURLTrackerInfoBarDelegate : public ConfirmInfoBarDelegate {
 public:
  GoogleURLTrackerInfoBarDelegate(TabContents* tab_contents,
                                  GoogleURLTracker* google_url_tracker,
                                  const GURL& new_google_url);

  // ConfirmInfoBarDelegate
  virtual bool Accept();
  virtual bool Cancel();
  virtual void InfoBarClosed();

 protected:
  virtual ~GoogleURLTrackerInfoBarDelegate();

  GoogleURLTracker* google_url_tracker_;
  const GURL new_google_url_;

 private:
  // ConfirmInfoBarDelegate
  virtual string16 GetMessageText() const;
  virtual int GetButtons() const;
  virtual string16 GetButtonLabel(InfoBarButton button) const;

  DISALLOW_COPY_AND_ASSIGN(GoogleURLTrackerInfoBarDelegate);
};

GoogleURLTrackerInfoBarDelegate::GoogleURLTrackerInfoBarDelegate(
    TabContents* tab_contents,
    GoogleURLTracker* google_url_tracker,
    const GURL& new_google_url)
    : ConfirmInfoBarDelegate(tab_contents),
      google_url_tracker_(google_url_tracker),
      new_google_url_(new_google_url) {
}

bool GoogleURLTrackerInfoBarDelegate::Accept() {
  google_url_tracker_->AcceptGoogleURL(new_google_url_);
  google_url_tracker_->RedoSearch();
  return true;
}

bool GoogleURLTrackerInfoBarDelegate::Cancel() {
  google_url_tracker_->CancelGoogleURL(new_google_url_);
  return true;
}

void GoogleURLTrackerInfoBarDelegate::InfoBarClosed() {
  google_url_tracker_->InfoBarClosed();
  delete this;
}

GoogleURLTrackerInfoBarDelegate::~GoogleURLTrackerInfoBarDelegate() {
}

string16 GoogleURLTrackerInfoBarDelegate::GetMessageText() const {
  // TODO(ukai): change new_google_url to google_base_domain?
  return l10n_util::GetStringFUTF16(IDS_GOOGLE_URL_TRACKER_INFOBAR_MESSAGE,
                                    UTF8ToUTF16(new_google_url_.spec()));
}

int GoogleURLTrackerInfoBarDelegate::GetButtons() const {
  return BUTTON_OK | BUTTON_CANCEL;
}

string16 GoogleURLTrackerInfoBarDelegate::GetButtonLabel(
    InfoBarButton button) const {
  return l10n_util::GetStringUTF16((button == BUTTON_OK) ?
                                   IDS_CONFIRM_MESSAGEBOX_YES_BUTTON_LABEL :
                                   IDS_CONFIRM_MESSAGEBOX_NO_BUTTON_LABEL);
}

InfoBarDelegate* CreateInfobar(TabContents* tab_contents,
                               GoogleURLTracker* google_url_tracker,
                               const GURL& new_google_url) {
  InfoBarDelegate* infobar = new GoogleURLTrackerInfoBarDelegate(tab_contents,
      google_url_tracker, new_google_url);
  tab_contents->AddInfoBar(infobar);
  return infobar;
}


// GoogleURLTracker -----------------------------------------------------------

const char GoogleURLTracker::kDefaultGoogleHomepage[] =
    "http://www.google.com/";
const char GoogleURLTracker::kSearchDomainCheckURL[] =
    "https://www.google.com/searchdomaincheck?format=domain&type=chrome";

GoogleURLTracker::GoogleURLTracker()
    : infobar_creator_(&CreateInfobar),
      google_url_(g_browser_process->local_state()->GetString(
          prefs::kLastKnownGoogleURL)),
      ALLOW_THIS_IN_INITIALIZER_LIST(runnable_method_factory_(this)),
      fetcher_id_(0),
      in_startup_sleep_(true),
      already_fetched_(false),
      need_to_fetch_(false),
      request_context_available_(!!Profile::GetDefaultRequestContext()),
      need_to_prompt_(false),
      controller_(NULL),
      infobar_(NULL) {
  registrar_.Add(this, NotificationType::DEFAULT_REQUEST_CONTEXT_AVAILABLE,
                 NotificationService::AllSources());

  net::NetworkChangeNotifier::AddObserver(this);

  // Configure to max_retries at most kMaxRetries times for 5xx errors.
  URLFetcherProtectEntry* protect =
      URLFetcherProtectManager::GetInstance()->Register(
          GURL(kSearchDomainCheckURL).host());
  static const int kMaxRetries = 5;
  protect->SetMaxRetries(kMaxRetries);

  // Because this function can be called during startup, when kicking off a URL
  // fetch can eat up 20 ms of time, we delay five seconds, which is hopefully
  // long enough to be after startup, but still get results back quickly.
  // Ideally, instead of this timer, we'd do something like "check if the
  // browser is starting up, and if so, come back later", but there is currently
  // no function to do this.
  static const int kStartFetchDelayMS = 5000;
  MessageLoop::current()->PostDelayedTask(FROM_HERE,
      runnable_method_factory_.NewRunnableMethod(
          &GoogleURLTracker::FinishSleep),
      kStartFetchDelayMS);
}

GoogleURLTracker::~GoogleURLTracker() {
  runnable_method_factory_.RevokeAll();
  net::NetworkChangeNotifier::RemoveObserver(this);
}

// static
GURL GoogleURLTracker::GoogleURL() {
  const GoogleURLTracker* const tracker =
      g_browser_process->google_url_tracker();
  return tracker ? tracker->google_url_ : GURL(kDefaultGoogleHomepage);
}

// static
void GoogleURLTracker::RequestServerCheck() {
  GoogleURLTracker* const tracker = g_browser_process->google_url_tracker();
  if (tracker)
    tracker->SetNeedToFetch();
}

// static
void GoogleURLTracker::RegisterPrefs(PrefService* prefs) {
  prefs->RegisterStringPref(prefs::kLastKnownGoogleURL,
                            kDefaultGoogleHomepage);
  prefs->RegisterStringPref(prefs::kLastPromptedGoogleURL, std::string());
}

// static
void GoogleURLTracker::GoogleURLSearchCommitted() {
  GoogleURLTracker* tracker = g_browser_process->google_url_tracker();
  if (tracker)
    tracker->SearchCommitted();
}

void GoogleURLTracker::SetNeedToFetch() {
  need_to_fetch_ = true;
  StartFetchIfDesirable();
}

void GoogleURLTracker::FinishSleep() {
  in_startup_sleep_ = false;
  StartFetchIfDesirable();
}

void GoogleURLTracker::StartFetchIfDesirable() {
  // Bail if a fetch isn't appropriate right now.  This function will be called
  // again each time one of the preconditions changes, so we'll fetch
  // immediately once all of them are met.
  //
  // See comments in header on the class, on RequestServerCheck(), and on the
  // various members here for more detail on exactly what the conditions are.
  if (in_startup_sleep_ || already_fetched_ || !need_to_fetch_ ||
      !request_context_available_)
    return;

  if (CommandLine::ForCurrentProcess()->HasSwitch(
      switches::kDisableBackgroundNetworking))
    return;

  already_fetched_ = true;
  fetcher_.reset(URLFetcher::Create(fetcher_id_, GURL(kSearchDomainCheckURL),
                                    URLFetcher::GET, this));
  ++fetcher_id_;
  // We don't want this fetch to affect existing state in the profile.  For
  // example, if a user has no Google cookies, this automatic check should not
  // cause one to be set, lest we alarm the user.
  fetcher_->set_load_flags(net::LOAD_DISABLE_CACHE |
                           net::LOAD_DO_NOT_SAVE_COOKIES);
  fetcher_->set_request_context(Profile::GetDefaultRequestContext());
  fetcher_->Start();
}

void GoogleURLTracker::OnURLFetchComplete(const URLFetcher* source,
                                          const GURL& url,
                                          const URLRequestStatus& status,
                                          int response_code,
                                          const ResponseCookies& cookies,
                                          const std::string& data) {
  // Delete the fetcher on this function's exit.
  scoped_ptr<URLFetcher> clean_up_fetcher(fetcher_.release());

  // Don't update the URL if the request didn't succeed.
  if (!status.is_success() || (response_code != 200)) {
    already_fetched_ = false;
    return;
  }

  // See if the response data was one we want to use, and if so, convert to the
  // appropriate Google base URL.
  std::string url_str;
  TrimWhitespace(data, TRIM_ALL, &url_str);

  if (!StartsWithASCII(url_str, ".google.", false))
    return;

  fetched_google_url_ = GURL("http://www" + url_str);
  GURL last_prompted_url(
      g_browser_process->local_state()->GetString(
          prefs::kLastPromptedGoogleURL));
  need_to_prompt_ = false;
  // On the very first run of Chrome, when we've never looked up the URL at all,
  // we should just silently switch over to whatever we get immediately.
  if (last_prompted_url.is_empty()) {
    AcceptGoogleURL(fetched_google_url_);
    // Set fetched_google_url_ as an initial value of last prompted URL.
    g_browser_process->local_state()->SetString(prefs::kLastPromptedGoogleURL,
                                                fetched_google_url_.spec());
    return;
  }

  if (fetched_google_url_ == last_prompted_url)
    return;
  if (fetched_google_url_ == google_url_) {
    // The user came back to their original location after having temporarily
    // moved.  Reset the prompted URL so we'll prompt again if they move again.
    g_browser_process->local_state()->SetString(prefs::kLastPromptedGoogleURL,
                                                fetched_google_url_.spec());
    return;
  }

  need_to_prompt_ = true;
}

void GoogleURLTracker::AcceptGoogleURL(const GURL& new_google_url) {
  google_url_ = new_google_url;
  g_browser_process->local_state()->SetString(prefs::kLastKnownGoogleURL,
                                              google_url_.spec());
  g_browser_process->local_state()->SetString(prefs::kLastPromptedGoogleURL,
                                              new_google_url.spec());
  NotificationService::current()->Notify(NotificationType::GOOGLE_URL_UPDATED,
                                         NotificationService::AllSources(),
                                         NotificationService::NoDetails());
  need_to_prompt_ = false;
}

void GoogleURLTracker::CancelGoogleURL(const GURL& new_google_url) {
  g_browser_process->local_state()->SetString(prefs::kLastPromptedGoogleURL,
                                              new_google_url.spec());
}

void GoogleURLTracker::InfoBarClosed() {
  registrar_.RemoveAll();
  controller_ = NULL;
  infobar_ = NULL;
  search_url_ = GURL();
}

void GoogleURLTracker::RedoSearch() {
  //  re-do the user's search on the new domain.
  DCHECK(controller_);
  url_canon::Replacements<char> replacements;
  replacements.SetHost(google_url_.host().data(),
                       url_parse::Component(0, google_url_.host().length()));
  search_url_ = search_url_.ReplaceComponents(replacements);
  if (search_url_.is_valid())
    controller_->tab_contents()->OpenURL(search_url_, GURL(), CURRENT_TAB,
                                         PageTransition::GENERATED);
}

void GoogleURLTracker::Observe(NotificationType type,
                               const NotificationSource& source,
                               const NotificationDetails& details) {
  switch (type.value) {
    case NotificationType::DEFAULT_REQUEST_CONTEXT_AVAILABLE:
      registrar_.Remove(this,
                        NotificationType::DEFAULT_REQUEST_CONTEXT_AVAILABLE,
                        NotificationService::AllSources());
      request_context_available_ = true;
      StartFetchIfDesirable();
      break;

    case NotificationType::NAV_ENTRY_PENDING:
      controller_ = Source<NavigationController>(source).ptr();
      search_url_ = controller_->pending_entry()->url();
      // We don't need to listen NAV_ENTRY_PENDING any more, until another
      // search is committed.
      registrar_.Remove(this, NotificationType::NAV_ENTRY_PENDING,
                        NotificationService::AllSources());
      // Start listening for the commit notification. We also need to listen
      // for the tab close command since that means the load will never
      // commit!
      registrar_.Add(this, NotificationType::NAV_ENTRY_COMMITTED,
                     Source<NavigationController>(controller_));
      registrar_.Add(this, NotificationType::TAB_CLOSED,
                     Source<NavigationController>(controller_));
      break;

    case NotificationType::NAV_ENTRY_COMMITTED:
      registrar_.RemoveAll();
      DCHECK(controller_);
      ShowGoogleURLInfoBarIfNecessary(controller_->tab_contents());
      break;

    case NotificationType::TAB_CLOSED:
      registrar_.RemoveAll();
      controller_ = NULL;
      infobar_ = NULL;
      break;

    default:
      NOTREACHED() << "Unknown notification received:" << type.value;
  }
}

void GoogleURLTracker::OnIPAddressChanged() {
  already_fetched_ = false;
  StartFetchIfDesirable();
}

void GoogleURLTracker::SearchCommitted() {
  if (!registrar_.IsEmpty() || (!need_to_prompt_ && !fetcher_.get()))
    return;
  registrar_.Add(this, NotificationType::NAV_ENTRY_PENDING,
                 NotificationService::AllSources());
}

void GoogleURLTracker::ShowGoogleURLInfoBarIfNecessary(
    TabContents* tab_contents) {
  if (!need_to_prompt_)
    return;
  DCHECK(!fetched_google_url_.is_empty());

  infobar_ = (*infobar_creator_)(tab_contents, this, fetched_google_url_);
}
