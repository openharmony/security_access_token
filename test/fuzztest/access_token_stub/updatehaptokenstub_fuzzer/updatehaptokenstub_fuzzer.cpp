/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "updatehaptokenstub_fuzzer.h"

#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#undef private
#include "service/accesstoken_manager_service.h"

using namespace std;
using namespace OHOS::Security::AccessToken;

namespace OHOS {
    bool UpdateHapTokenStubFuzzTest(const uint8_t* data, size_t size)
    {
        if ((data == nullptr) || (size == 0)) {
            return false;
        }
        AccessTokenID TOKENID = static_cast<AccessTokenID>(size);
        std::string testName(reinterpret_cast<const char*>(data), size);
        PermissionDef TestPermDef = {.permissionName = testName,
            .bundleName = testName,
            .grantMode = 1,
            .availableLevel = APL_NORMAL,
            .label = testName,
            .labelId = 1,
            .description = testName,
            .descriptionId = 1};
        PermissionStateFull TestState = {.permissionName = testName,
            .isGeneral = true,
            .resDeviceID = {testName},
            .grantStatus = {PermissionState::PERMISSION_GRANTED},
            .grantFlags = {1}};
        HapPolicyParams policy = {.apl = APL_NORMAL,
            .domain = testName,
            .permList = {TestPermDef},
            .permStateList = {TestState}};
        int32_t apiVersion = 8;
        HapPolicyParcel hapPolicyParcel;
        hapPolicyParcel.hapPolicyParameter = policy;
        MessageParcel datas;
        datas.WriteInterfaceToken(IAccessTokenManager::GetDescriptor());
        if (!datas.WriteUint32(TOKENID)) {
            return false;
        }
        if (!datas.WriteBool(false)) {
            return false;
        }
        if (!datas.WriteString(testName)) {
            return false;
        }
        if (!datas.WriteInt32(apiVersion)) {
            return false;
        }
        if (!datas.WriteParcelable(&hapPolicyParcel)) {
            return false;
        }
        uint32_t code = static_cast<uint32_t>(IAccessTokenManager::AccessTokenInterfaceCode::UPDATE_HAP_TOKEN);
        MessageParcel reply;
        MessageOption option;
        DelayedSingleton<AccessTokenManagerService>::GetInstance()->OnRemoteRequest(code, datas, reply, option);
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::UpdateHapTokenStubFuzzTest(data, size);
    return 0;
}
