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

#include "setmutepolicystub_fuzzer.h"

#include <string>
#include <thread>
#include <vector>
#undef private
#include "accesstoken_kit.h"
#include "i_privacy_manager.h"
#include "privacy_manager_service.h"
#include "nativetoken_kit.h"
#include "securec.h"
#include "token_setproc.h"

using namespace std;
using namespace OHOS::Security::AccessToken;
const int CONSTANTS_NUMBER_TWO = 2;
static const int32_t ROOT_UID = 0;

namespace OHOS {
const uint8_t *g_baseFuzzData = nullptr;
size_t g_baseFuzzSize = 0;
size_t g_baseFuzzPos = 0;
    void GetNativeToken()
    {
        uint64_t tokenId;
        const char **perms = new const char *[1];
        perms[0] = "ohos.permission.GET_SENSITIVE_PERMISSIONS"; // 3 means the third permission

        NativeTokenInfoParams infoInstance = {
            .dcapsNum = 0,
            .permsNum = 1,
            .aclsNum = 0,
            .dcaps = nullptr,
            .perms = perms,
            .acls = nullptr,
            .processName = "getpermissionsstatusstub_fuzzer_test",
            .aplStr = "system_core",
        };

        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        AccessTokenKit::ReloadNativeTokenInfo();
        delete[] perms;
    }

    /*
    * describe: get data from outside untrusted data(g_data) which size is according to sizeof(T)
    * tips: only support basic type
    */
    template<class T> T GetData()
    {
        T object {};
        size_t objectSize = sizeof(object);
        if (g_baseFuzzData == nullptr || objectSize > g_baseFuzzSize - g_baseFuzzPos) {
            return object;
        }
        errno_t ret = memcpy_s(&object, objectSize, g_baseFuzzData + g_baseFuzzPos, objectSize);
        if (ret != EOK) {
            return {};
        }
        g_baseFuzzPos += objectSize;
        return object;
    }

    bool SetMutePolicyStubFuzzTest(const uint8_t* data, size_t size)
    {
        if ((data == nullptr) || (size == 0)) {
            return false;
        }
        GetNativeToken();
        g_baseFuzzData = data;
        g_baseFuzzSize = size;
        g_baseFuzzPos = 0;
        if (size > sizeof(uint32_t) + sizeof(bool)) {
            uint32_t policyType = GetData<uint32_t>();
            uint32_t callerType = GetData<uint32_t>();
            bool isMute = (GetData<char>() % CONSTANTS_NUMBER_TWO) == 0;

            MessageParcel datas;
            datas.WriteInterfaceToken(IPrivacyManager::GetDescriptor());
            if (!datas.WriteUint32(policyType)) {
                return false;
            }
            if (!datas.WriteUint32(callerType)) {
                return false;
            }
            if (!datas.WriteBool(isMute)) {
                return false;
            }

            uint32_t code = static_cast<uint32_t>(
                PrivacyInterfaceCode::SET_MUTE_POLICY);

            MessageParcel reply;
            MessageOption option;
            bool enable = ((size % CONSTANTS_NUMBER_TWO) == 0);
            if (enable) {
                setuid(CONSTANTS_NUMBER_TWO);
            }
            DelayedSingleton<PrivacyManagerService>::GetInstance()->OnRemoteRequest(code, datas, reply, option);
            setuid(ROOT_UID);
        }
        return true;
    }
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::SetMutePolicyStubFuzzTest(data, size);
    return 0;
}