// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/policy/device_local_account_policy_store.h"

#include "base/bind.h"
#include "chrome/browser/policy/device_management_service.h"
#include "chrome/browser/policy/proto/cloud_policy.pb.h"
#include "chrome/browser/policy/proto/device_management_backend.pb.h"
#include "chromeos/dbus/session_manager_client.h"

namespace em = enterprise_management;

namespace policy {

DeviceLocalAccountPolicyStore::DeviceLocalAccountPolicyStore(
    const std::string& account_id,
    chromeos::SessionManagerClient* session_manager_client,
    chromeos::DeviceSettingsService* device_settings_service)
    : account_id_(account_id),
      session_manager_client_(session_manager_client),
      device_settings_service_(device_settings_service),
      ALLOW_THIS_IN_INITIALIZER_LIST(weak_factory_(this)) {}

DeviceLocalAccountPolicyStore::~DeviceLocalAccountPolicyStore() {}

void DeviceLocalAccountPolicyStore::Load() {
  weak_factory_.InvalidateWeakPtrs();
  session_manager_client_->RetrieveDeviceLocalAccountPolicy(
      account_id_,
      base::Bind(&DeviceLocalAccountPolicyStore::ValidateLoadedPolicyBlob,
                 weak_factory_.GetWeakPtr()));
}

void DeviceLocalAccountPolicyStore::Store(
    const em::PolicyFetchResponse& policy) {
  weak_factory_.InvalidateWeakPtrs();
  CheckKeyAndValidate(
      make_scoped_ptr(new em::PolicyFetchResponse(policy)),
      base::Bind(&DeviceLocalAccountPolicyStore::StoreValidatedPolicy,
                 weak_factory_.GetWeakPtr()));
}

void DeviceLocalAccountPolicyStore::ValidateLoadedPolicyBlob(
    const std::string& policy_blob) {
  if (policy_blob.empty()) {
    status_ = CloudPolicyStore::STATUS_LOAD_ERROR;
    NotifyStoreError();
  } else {
    scoped_ptr<em::PolicyFetchResponse> policy(new em::PolicyFetchResponse());
    if (policy->ParseFromString(policy_blob)) {
      CheckKeyAndValidate(
          policy.Pass(),
          base::Bind(&DeviceLocalAccountPolicyStore::UpdatePolicy,
                     weak_factory_.GetWeakPtr()));
    } else {
      status_ = CloudPolicyStore::STATUS_PARSE_ERROR;
      NotifyStoreError();
    }
  }
}

void DeviceLocalAccountPolicyStore::UpdatePolicy(
    UserCloudPolicyValidator* validator) {
  validation_status_ = validator->status();
  if (!validator->success()) {
    status_ = STATUS_VALIDATION_ERROR;
    NotifyStoreError();
    return;
  }

  InstallPolicy(validator->policy_data().Pass(), validator->payload().Pass());
  status_ = STATUS_OK;
  NotifyStoreLoaded();
}

void DeviceLocalAccountPolicyStore::StoreValidatedPolicy(
    UserCloudPolicyValidator* validator) {
  if (!validator->success()) {
    status_ = CloudPolicyStore::STATUS_VALIDATION_ERROR;
    validation_status_ = validator->status();
    NotifyStoreError();
    return;
  }

  std::string policy_blob;
  if (!validator->policy()->SerializeToString(&policy_blob)) {
    status_ = CloudPolicyStore::STATUS_SERIALIZE_ERROR;
    NotifyStoreError();
    return;
  }

  session_manager_client_->StoreDeviceLocalAccountPolicy(
      account_id_,
      policy_blob,
      base::Bind(&DeviceLocalAccountPolicyStore::HandleStoreResult,
                 weak_factory_.GetWeakPtr()));
}

void DeviceLocalAccountPolicyStore::HandleStoreResult(bool success) {
  if (!success) {
    status_ = CloudPolicyStore::STATUS_STORE_ERROR;
    NotifyStoreError();
  } else {
    Load();
  }
}

void DeviceLocalAccountPolicyStore::CheckKeyAndValidate(
    scoped_ptr<em::PolicyFetchResponse> policy,
    const UserCloudPolicyValidator::CompletionCallback& callback) {
  device_settings_service_->GetOwnershipStatusAsync(
      base::Bind(&DeviceLocalAccountPolicyStore::Validate,
                 weak_factory_.GetWeakPtr(),
                 base::Passed(policy.Pass()),
                 callback));
}

void DeviceLocalAccountPolicyStore::Validate(
    scoped_ptr<em::PolicyFetchResponse> policy_response,
    const UserCloudPolicyValidator::CompletionCallback& callback,
    chromeos::DeviceSettingsService::OwnershipStatus ownership_status,
    bool is_owner) {
  DCHECK_NE(chromeos::DeviceSettingsService::OWNERSHIP_UNKNOWN,
            ownership_status);
  chromeos::OwnerKey* key = device_settings_service_->GetOwnerKey();
  if (!key->public_key()) {
    status_ = CloudPolicyStore::STATUS_BAD_STATE;
    NotifyStoreLoaded();
    return;
  }

  scoped_ptr<UserCloudPolicyValidator> validator(
      UserCloudPolicyValidator::Create(policy_response.Pass()));
  validator->ValidateUsername(account_id_);
  validator->ValidatePolicyType(dm_protocol::kChromePublicAccountPolicyType);
  validator->ValidateAgainstCurrentPolicy(policy(), false);
  validator->ValidatePayload();
  validator->ValidateSignature(*key->public_key(), false);
  validator.release()->StartValidation(callback);
}

}  // namespace policy