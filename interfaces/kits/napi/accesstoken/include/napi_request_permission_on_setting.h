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
#ifndef INTERFACES_ACCESSTOKEN_KITS_NAPI_REQUEST_PERMISSION_ON_SETTING_H
#define INTERFACES_ACCESSTOKEN_KITS_NAPI_REQUEST_PERMISSION_ON_SETTING_H

#include "napi_context_common.h"
#include "permission_grant_info.h"
#include "ui_content.h"
#include "ui_extension_context.h"

namespace OHOS {
namespace Security {
namespace AccessToken {
struct RequestPermOnSettingAsyncContext : public AtManagerAsyncWorkData {
    explicit RequestPermOnSettingAsyncContext(napi_env env) : AtManagerAsyncWorkData(env)
    {
        this->env = env;
    }

    AccessTokenID tokenId = 0;
    int32_t result = RET_SUCCESS;
    PermissionGrantInfo info;
    int32_t resultCode = -1;

    std::vector<std::string> permissionList;
    napi_value requestResult = nullptr;
    int32_t errorCode = -1;
    std::vector<int32_t> stateList;

    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext;
    std::shared_ptr<AbilityRuntime::UIExtensionContext> uiExtensionContext;
    bool uiAbilityFlag = false;
    bool releaseFlag = false;
};

struct RequestOnSettingAsyncContextHandle {
    explicit RequestOnSettingAsyncContextHandle(std::shared_ptr<RequestPermOnSettingAsyncContext>& requestAsyncContext)
    {
        asyncContextPtr = requestAsyncContext;
    }

    std::shared_ptr<RequestPermOnSettingAsyncContext> asyncContextPtr;
};

class PermissonOnSettingUICallback {
public:
    explicit PermissonOnSettingUICallback(const std::shared_ptr<RequestPermOnSettingAsyncContext>& reqContext);
    ~PermissonOnSettingUICallback();
    void SetSessionId(int32_t sessionId);
    void OnRelease(int32_t releaseCode);
    void OnResult(int32_t resultCode, const OHOS::AAFwk::Want& result);
    void OnReceive(const OHOS::AAFwk::WantParams& request);
    void OnError(int32_t code, const std::string& name, const std::string& message);
    void OnRemoteReady(const std::shared_ptr<OHOS::Ace::ModalUIExtensionProxy>& uiProxy);
    void OnDestroy();
    void ReleaseHandler(int32_t code);

private:
    int32_t sessionId_ = 0;
    std::shared_ptr<RequestPermOnSettingAsyncContext> reqContext_ = nullptr;
};

struct PermissonOnSettingResultCallback {
    int32_t jsCode;
    std::vector<int32_t> stateList;
    std::shared_ptr<RequestPermOnSettingAsyncContext> data = nullptr;
};

class NapiRequestPermissionOnSetting {
public:
    static napi_value RequestPermissionOnSetting(napi_env env, napi_callback_info info);

private:
    static bool ParseRequestPermissionOnSetting(const napi_env& env, const napi_callback_info& cbInfo,
        std::shared_ptr<RequestPermOnSettingAsyncContext>& asyncContext);
    static void RequestPermissionOnSettingExecute(napi_env env, void* data);
    static void RequestPermissionOnSettingComplete(napi_env env, napi_status status, void* data);
};
} // namespace AccessToken
} // namespace Security
} // namespace OHOS
#endif /* INTERFACES_ACCESSTOKEN_KITS_NAPI_REQUEST_PERMISSION_ON_SETTING_H */