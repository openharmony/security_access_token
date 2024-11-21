/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "callback_manager.h"

#include <future>
#include <thread>
#include <datetime_ex.h>
#include <pthread.h>

#include "access_token.h"
#include "access_token_error.h"
#include "callback_death_recipients.h"


namespace OHOS {
namespace Security {
namespace AccessToken {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_ACCESSTOKEN, "CallbackManager"};
static const uint32_t MAX_CALLBACK_SIZE = 1024;
#ifndef RESOURCESCHEDULE_FFRT_ENABLE
static const int MAX_PTHREAD_NAME_LEN = 15; // pthread name max length
#endif
std::recursive_mutex g_instanceMutex;
}

CallbackManager& CallbackManager::GetInstance()
{
    static CallbackManager* instance = nullptr;
    if (instance == nullptr) {
        std::lock_guard<std::recursive_mutex> lock(g_instanceMutex);
        if (instance == nullptr) {
            instance = new CallbackManager();
        }
    }
    return *instance;
}

CallbackManager::CallbackManager() : callbackDeathRecipient_(sptr<IRemoteObject::DeathRecipient>(
    new (std::nothrow) PermStateCallbackDeathRecipient()))
{
}

CallbackManager::~CallbackManager()
{
}

int32_t CallbackManager::AddCallback(const PermStateChangeScope& scopeRes, const sptr<IRemoteObject>& callback)
{
    if (callback == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Input is nullptr");
        return AccessTokenError::ERR_PARAM_INVALID;
    }
    auto callbackScopePtr = std::make_shared<PermStateChangeScope>(scopeRes);

#ifdef RESOURCESCHEDULE_FFRT_ENABLE
    std::lock_guard<ffrt::mutex> lock(mutex_);
#else
    std::lock_guard<std::mutex> lock(mutex_);
#endif
    if (callbackInfoList_.size() >= MAX_CALLBACK_SIZE) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Callback size has reached limitation");
        return AccessTokenError::ERR_CALLBACKS_EXCEED_LIMITATION;
    }
    int32_t ret = callback->AddDeathRecipient(callbackDeathRecipient_);
    if (ret != ERR_NONE) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "add death recipient failed ret is %{public}d", ret);
    }

    CallbackRecord recordInstance;
    recordInstance.callbackObject_ = callback;
    recordInstance.scopePtr_ = callbackScopePtr;

    callbackInfoList_.emplace_back(recordInstance);

    ACCESSTOKEN_LOG_INFO(LABEL, "RecordInstance is added");
    return RET_SUCCESS;
}

int32_t CallbackManager::RemoveCallback(const sptr<IRemoteObject>& callback)
{
    if (callback == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Callback is nullptr.");
        return AccessTokenError::ERR_PARAM_INVALID;
    }

#ifdef RESOURCESCHEDULE_FFRT_ENABLE
    std::lock_guard<ffrt::mutex> lock(mutex_);
#else
    std::lock_guard<std::mutex> lock(mutex_);
#endif

    for (auto it = callbackInfoList_.begin(); it != callbackInfoList_.end(); ++it) {
        if (callback == (*it).callbackObject_) {
            ACCESSTOKEN_LOG_INFO(LABEL, "Find callback");
            if (callbackDeathRecipient_ != nullptr) {
                callback->RemoveDeathRecipient(callbackDeathRecipient_);
            }
            (*it).callbackObject_ = nullptr;
            callbackInfoList_.erase(it);
            break;
        }
    }
    ACCESSTOKEN_LOG_INFO(LABEL, "CallbackInfoList_ %{public}u", (uint32_t)callbackInfoList_.size());
    return RET_SUCCESS;
}

bool CallbackManager::CalledAccordingToTokenIdLlist(
    const std::vector<AccessTokenID>& tokenIDList, AccessTokenID tokenID)
{
    if (tokenIDList.empty()) {
        return true;
    }
    return std::any_of(tokenIDList.begin(), tokenIDList.end(),
        [tokenID](AccessTokenID id) { return id == tokenID; });
}

bool CallbackManager::CalledAccordingToPermLlist(const std::vector<std::string>& permList, const std::string& permName)
{
    if (permList.empty()) {
        return true;
    }
    return std::any_of(permList.begin(), permList.end(),
        [permName](const std::string& perm) { return perm == permName; });
}

void CallbackManager::ExecuteAllCallback(std::vector<sptr<IRemoteObject>>& list, AccessTokenID tokenID,
    const std::string& permName, int32_t changeType)
{
    for (auto it = list.begin(); it != list.end(); ++it) {
#ifdef RESOURCESCHEDULE_FFRT_ENABLE
        auto callbackSingle = [it, tokenID, permName, changeType]() {
            sptr<IPermissionStateCallback> callback = new PermissionStateChangeCallbackProxy(*it);
            if (callback != nullptr) {
                ACCESSTOKEN_LOG_INFO(LABEL, "Callback execute");
                PermStateChangeInfo resInfo;
                resInfo.permStateChangeType = changeType;
                resInfo.permissionName = permName;
                resInfo.tokenID = tokenID;
                callback->PermStateChangeCallback(resInfo);
                ACCESSTOKEN_LOG_INFO(LABEL, "Callback execute end");
            }
        };
        ffrt::submit(callbackSingle, {}, {}, ffrt::task_attr().qos(ffrt::qos_default));
#else
        sptr<IPermissionStateCallback> callback = new PermissionStateChangeCallbackProxy(*it);
        if (callback != nullptr) {
            ACCESSTOKEN_LOG_INFO(LABEL, "Callback execute");
            PermStateChangeInfo resInfo;
            resInfo.permStateChangeType = changeType;
            resInfo.permissionName = permName;
            resInfo.tokenID = tokenID;
            callback->PermStateChangeCallback(resInfo);
        }
#endif
    }
#ifdef RESOURCESCHEDULE_FFRT_ENABLE
    ffrt::wait();
#endif
}

void CallbackManager::GetCallbackObjectList(AccessTokenID tokenID, const std::string& permName,
    std::vector<sptr<IRemoteObject>>& list)
{
#ifdef RESOURCESCHEDULE_FFRT_ENABLE
    std::lock_guard<ffrt::mutex> lock(mutex_);
#else
    std::lock_guard<std::mutex> lock(mutex_);
#endif
    for (auto it = callbackInfoList_.begin(); it != callbackInfoList_.end(); ++it) {
        std::shared_ptr<PermStateChangeScope> scopePtr = (*it).scopePtr_;
        if (scopePtr == nullptr) {
            ACCESSTOKEN_LOG_ERROR(LABEL, "ScopePtr is nullptr");
            continue;
        }
        if (!CalledAccordingToTokenIdLlist(scopePtr->tokenIDs, tokenID) ||
            !CalledAccordingToPermLlist(scopePtr->permList, permName)) {
                ACCESSTOKEN_LOG_DEBUG(LABEL,
                    "tokenID is %{public}u, permName is  %{public}s", tokenID, permName.c_str());
                continue;
        }
        list.emplace_back((*it).callbackObject_);
    }
}

void CallbackManager::ExecuteCallbackAsync(AccessTokenID tokenID, const std::string& permName, int32_t changeType)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "Entry");
    auto callbackStart = [this, tokenID, permName, changeType]() {
        ACCESSTOKEN_LOG_INFO(LABEL, "CallbackStart");
#ifndef RESOURCESCHEDULE_FFRT_ENABLE
        std::string name = "AtmCallback";
        pthread_setname_np(pthread_self(), name.substr(0, MAX_PTHREAD_NAME_LEN).c_str());
#endif
        std::vector<sptr<IRemoteObject>> list;
        this->GetCallbackObjectList(tokenID, permName, list);
        this->ExecuteAllCallback(list, tokenID, permName, changeType);
    };

#ifdef RESOURCESCHEDULE_FFRT_ENABLE
    std::string taskName = "AtmCallback";
    ffrt::submit_h(callbackStart, {}, {},
        ffrt::task_attr().qos(ffrt::qos_default).name(taskName.c_str()));
#else
    std::packaged_task<void()> callbackTask(callbackStart);
    std::make_unique<std::thread>(std::move(callbackTask))->detach();
#endif
    ACCESSTOKEN_LOG_DEBUG(LABEL, "The callback execution is complete");
}
} // namespace AccessToken
} // namespace Security
} // namespace OHOS
