// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Chromium settings and storage represent user-selected preferences and
// information and MUST not be extracted, overwritten or modified except
// through Chromium defined APIs.

#ifndef CHROME_BROWSER_WEBDATA_WEB_DATA_REQUEST_MANAGER_H__
#define CHROME_BROWSER_WEBDATA_WEB_DATA_REQUEST_MANAGER_H__

#include <map>

#include "base/memory/ref_counted.h"
#include "base/synchronization/lock.h"
#include "chrome/browser/api/webdata/web_data_results.h"
#include "chrome/browser/api/webdata/web_data_service_base.h"
#include "chrome/browser/api/webdata/web_data_service_consumer.h"

class MessageLoop;
class WebDataService;
class WebDataServiceConsumer;
class WebDataRequestManager;

//////////////////////////////////////////////////////////////////////////////
//
// Webdata requests
//
// Every request is processed using a request object. The object contains
// both the request parameters and the results.
//////////////////////////////////////////////////////////////////////////////
class WebDataRequest {
 public:
  WebDataRequest(WebDataService* service,
                 WebDataServiceConsumer* consumer,
                 WebDataRequestManager* manager);

  virtual ~WebDataRequest();

  WebDataServiceBase::Handle GetHandle() const;

  // Retrieves the |consumer_| set in the constructor.
  WebDataServiceConsumer* GetConsumer() const;

  // Returns |true| if the request was cancelled via the |Cancel()| method.
  bool IsCancelled() const;

  // This can be invoked from any thread. From this point we assume that
  // our consumer_ reference is invalid.
  void Cancel();

  // Invoked by the service when this request has been completed.
  // This will notify the service in whatever thread was used to create this
  // request.
  void RequestComplete();

  // The result is owned by the request.
  void SetResult(WDTypedResult* r);
  const WDTypedResult* GetResult() const;

 private:
  // Used to notify service of request completion.
  scoped_refptr<WebDataService> service_;

  // Tracks loop that the request originated on.
  MessageLoop* message_loop_;

  // Identifier for this request.
  WebDataServiceBase::Handle handle_;

  // A lock to protect against simultaneous cancellations of the request.
  // Cancellation affects both the |cancelled_| flag and |consumer_|.
  mutable base::Lock cancel_lock_;
  bool cancelled_;

  // The originator of the service request.
  WebDataServiceConsumer* consumer_;

  WDTypedResult* result_;

  DISALLOW_COPY_AND_ASSIGN(WebDataRequest);
};

//////////////////////////////////////////////////////////////////////////////
//
// Webdata request templates
//
// Internally we use instances of the following template to represent
// requests.
//////////////////////////////////////////////////////////////////////////////

template <class T>
class GenericRequest : public WebDataRequest {
 public:
  GenericRequest(WebDataService* service,
                 WebDataServiceConsumer* consumer,
                 WebDataRequestManager* manager,
                 const T& arg)
      : WebDataRequest(service, consumer, manager),
        arg_(arg) {
  }

  virtual ~GenericRequest() {
  }

  const T& arg() const { return arg_; }

 private:
  T arg_;
};

template <class T, class U>
class GenericRequest2 : public WebDataRequest {
 public:
  GenericRequest2(WebDataService* service,
                  WebDataServiceConsumer* consumer,
                  WebDataRequestManager* manager,
                  const T& arg1,
                  const U& arg2)
      : WebDataRequest(service, consumer, manager),
        arg1_(arg1),
        arg2_(arg2) {
  }

  virtual ~GenericRequest2() { }

  const T& arg1() const { return arg1_; }

  const U& arg2() const { return arg2_; }

 private:
  T arg1_;
  U arg2_;
};

//////////////////////////////////////////////////////////////////////////////
//
// Webdata Request Manager
//
// Tracks all WebDataRequests for a WebDataService.
//
// Note: This is an internal interface, not to be used outside of webdata/
//////////////////////////////////////////////////////////////////////////////
class WebDataRequestManager {
 public:
  WebDataRequestManager();

  ~WebDataRequestManager();

  // Cancel any pending request.
  void CancelRequest(WebDataServiceBase::Handle h);

  // Invoked by request implementations when a request has been processed.
  void RequestCompleted(WebDataServiceBase::Handle h);

  // Register the request as a pending request.
  void RegisterRequest(WebDataRequest* request);

  // Return the next request handle.
  int GetNextRequestHandle();

 private:
  // A lock to protect pending requests and next request handle.
  base::Lock pending_lock_;

  // Next handle to be used for requests. Incremented for each use.
  WebDataServiceBase::Handle next_request_handle_;

  typedef std::map<WebDataServiceBase::Handle, WebDataRequest*> RequestMap;
  RequestMap pending_requests_;

  DISALLOW_COPY_AND_ASSIGN(WebDataRequestManager);
};

#endif  // CHROME_BROWSER_WEBDATA_WEB_DATA_REQUEST_MANAGER_H__
