/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "privacy_session_manager_proxy.h"

#include "accesstoken_common_log.h"

namespace OHOS {
namespace Security {
namespace AccessToken {

sptr<IRemoteObject> PrivacySessionManagerProxy::GetSceneSessionManager()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE(PRI_DOMAIN, PRI_TAG, "WriteInterfaceToken failed");
        return nullptr;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LOGE(PRI_DOMAIN, PRI_TAG, "Remote service is null.");
        return nullptr;
    }
    auto ret = remote->SendRequest(
        static_cast<uint32_t>(SessionManagerServiceMessage::TRANS_ID_GET_SCENE_SESSION_MANAGER),
        data, reply, option);
    if (ret != ERR_NONE) {
        LOGE(PRI_DOMAIN, PRI_TAG, "SendRequest failed, errorCode %{public}d", ret);
        return nullptr;
    }

    return reply.ReadRemoteObject();
}

sptr<IRemoteObject> PrivacySessionManagerProxy::GetSceneSessionManagerLite()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE(PRI_DOMAIN, PRI_TAG, "WriteInterfaceToken failed");
        return nullptr;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LOGE(PRI_DOMAIN, PRI_TAG, "Remote service is null.");
        return nullptr;
    }
    auto ret = remote->SendRequest(
        static_cast<uint32_t>(SessionManagerServiceMessage::TRANS_ID_GET_SCENE_SESSION_MANAGER_LITE),
        data, reply, option);
    if (ret != ERR_NONE) {
        LOGE(PRI_DOMAIN, PRI_TAG, "SendRequest failed, errorCode %{public}d", ret);
        return nullptr;
    }

    return reply.ReadRemoteObject();
}
}
}
}