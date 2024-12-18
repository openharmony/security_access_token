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

#include "setremotehaptokeninfostub_fuzzer.h"

#include <string>
#include <thread>
#include <vector>
#undef private
#include "access_token.h"
#include "accesstoken_fuzzdata.h"
#include "accesstoken_info_manager.h"
#include "accesstoken_kit.h"
#include "accesstoken_manager_service.h"
#include "i_accesstoken_manager.h"
#include "permission_state_full.h"
#include "token_setproc.h"

using namespace std;
using namespace OHOS::Security::AccessToken;
#ifdef TOKEN_SYNC_ENABLE
const int CONSTANTS_NUMBER_TWO = 2;
#endif

namespace OHOS {
    #ifdef TOKEN_SYNC_ENABLE
    void ConstructorParam(
        AccessTokenFuzzData& fuzzData, AccessTokenID tokenId, HapTokenInfoForSyncParcel& hapSyncParcel)
    {
        std::string permissionName(fuzzData.GenerateStochasticString());
        HapTokenInfo baseInfo = {
            .apl = APL_NORMAL,
            .ver = 1,
            .userID = 1,
            .bundleName = fuzzData.GenerateStochasticString(),
            .instIndex = 1,
            .appID = fuzzData.GenerateStochasticString(),
            .deviceID = fuzzData.GenerateStochasticString(),
            .tokenID = tokenId,
            .tokenAttr = 0
        };
        PermissionStateFull infoManagerTestState = {
            .grantFlags = {PermissionFlag::PERMISSION_SYSTEM_FIXED},
            .grantStatus = {PermissionState::PERMISSION_GRANTED},
            .isGeneral = true,
            .permissionName = permissionName,
            .resDeviceID = {fuzzData.GenerateStochasticString()}};
        PermissionStateFull infoManagerTestState2 = {
            .grantFlags = {PermissionFlag::PERMISSION_USER_SET},
            .grantStatus = {PermissionState::PERMISSION_DENIED},
            .isGeneral = true,
            .permissionName = permissionName,
            .resDeviceID = {fuzzData.GenerateStochasticString()}};
        std::vector<PermissionStateFull> permStateList;
        permStateList.emplace_back(infoManagerTestState);
        HapTokenInfoForSync remoteTokenInfo = {
            .baseInfo = baseInfo,
            .permStateList = permStateList
        };
        hapSyncParcel.hapTokenInfoForSyncParams = remoteTokenInfo;
    }
    #endif

    bool SetRemoteHapTokenInfoStubFuzzTest(const uint8_t* data, size_t size)
    {
    #ifdef TOKEN_SYNC_ENABLE
        if ((data == nullptr) || (size == 0)) {
            return false;
        }

        AccessTokenFuzzData fuzzData(data, size);
        AccessTokenID tokenId = fuzzData.GetData<AccessTokenID>();
        HapTokenInfoForSyncParcel hapSyncParcel;
        ConstructorParam(fuzzData, tokenId, hapSyncParcel);

        MessageParcel datas;
        datas.WriteInterfaceToken(IAccessTokenManager::GetDescriptor());
        if (!datas.WriteString(fuzzData.GenerateStochasticString())) {
            return false;
        }
        if (!datas.WriteParcelable(&hapSyncParcel)) {
            return false;
        }

        uint32_t code = static_cast<uint32_t>(
            AccessTokenInterfaceCode::SET_REMOTE_HAP_TOKEN_INFO);

        MessageParcel reply;
        MessageOption option;
        bool enable = ((size % CONSTANTS_NUMBER_TWO) == 0);
        if (enable) {
            AccessTokenID accesstoken = AccessTokenKit::GetNativeTokenId("token_sync_service");
            SetSelfTokenID(accesstoken);
            AccessTokenInfoManager::GetInstance().Init();
        }
        DelayedSingleton<AccessTokenManagerService>::GetInstance()->OnRemoteRequest(code, datas, reply, option);
        AccessTokenID hdcd = AccessTokenKit::GetNativeTokenId("hdcd");
        SetSelfTokenID(hdcd);

        return true;
    #else
        return true;
    #endif
    }
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::SetRemoteHapTokenInfoStubFuzzTest(data, size);
    return 0;
}
