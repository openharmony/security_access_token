/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "app_manager_access_proxy.h"
#include "accesstoken_log.h"

namespace OHOS {
namespace Security {
namespace AccessToken {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_ACCESSTOKEN, "AppManagerAccessProxy"};
static constexpr int32_t ERROR = -1;
constexpr int32_t CYCLE_LIMIT = 1000;
}

sptr<IAmsMgr> AppManagerAccessProxy::GetAmsMgr()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return nullptr;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Remote service is null.");
        return nullptr;
    }
    int32_t error = remote->SendRequest(
        static_cast<uint32_t>(IAppMgr::Message::APP_GET_MGR_INSTANCE), data, reply, option);
    if (error != ERR_NONE) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "GetAmsMgr failed, error: %{public}d", error);
        return nullptr;
    }
    sptr<IRemoteObject> object = reply.ReadRemoteObject();
    sptr<IAmsMgr> amsMgr = new AmsManagerAccessProxy(object);
    if (!amsMgr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Ability manager service instance is nullptr. ");
        return nullptr;
    }
    return amsMgr;
}

int32_t AppManagerAccessProxy::RegisterApplicationStateObserver(const sptr<IApplicationStateObserver>& observer,
    const std::vector<std::string>& bundleNameList)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "WriteInterfaceToken failed");
        return ERROR;
    }
    if (!data.WriteRemoteObject(observer->AsObject())) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Observer write failed.");
        return ERROR;
    }
    if (!data.WriteStringVector(bundleNameList)) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "BundleNameList write failed.");
        return ERROR;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Remote service is null.");
        return ERROR;
    }
    int32_t error = remote->SendRequest(
        static_cast<uint32_t>(IAppMgr::Message::REGISTER_APPLICATION_STATE_OBSERVER), data, reply, option);
    if (error != ERR_NONE) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "RegisterAppStatus failed, error: %{public}d", error);
        return ERROR;
    }
    return reply.ReadInt32();
}

int32_t AppManagerAccessProxy::UnregisterApplicationStateObserver(
    const sptr<IApplicationStateObserver>& observer)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "WriteInterfaceToken failed");
        return ERROR;
    }
    if (!data.WriteRemoteObject(observer->AsObject())) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Observer write failed.");
        return ERROR;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Remote service is null.");
        return ERROR;
    }
    int32_t error = remote->SendRequest(
        static_cast<uint32_t>(IAppMgr::Message::UNREGISTER_APPLICATION_STATE_OBSERVER), data, reply, option);
    if (error != ERR_NONE) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Set microphoneMute failed, error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t AppManagerAccessProxy::GetForegroundApplications(std::vector<AppStateData>& list)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "WriteInterfaceToken failed");
        return ERROR;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Remote service is null.");
        return ERROR;
    }
    int32_t error = remote->SendRequest(
        static_cast<uint32_t>(IAppMgr::Message::GET_FOREGROUND_APPLICATIONS), data, reply, option);
    if (error != ERR_NONE) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "GetForegroundApplications failed, error: %{public}d", error);
        return error;
    }
    uint32_t infoSize = reply.ReadUint32();
    if (infoSize > CYCLE_LIMIT) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "InfoSize is too large");
        return ERROR;
    }
    for (uint32_t i = 0; i < infoSize; i++) {
        std::unique_ptr<AppStateData> info(reply.ReadParcelable<AppStateData>());
        if (info != nullptr) {
            list.emplace_back(*info);
        }
    }
    return reply.ReadInt32();
}
} // namespace AccessToken
} // namespace Security
} // namespace OHOS
