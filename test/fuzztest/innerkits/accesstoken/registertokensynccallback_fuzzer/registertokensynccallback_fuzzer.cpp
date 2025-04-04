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

#include "registertokensynccallback_fuzzer.h"

#include "accesstoken_kit.h"
#include "token_setproc.h"
#include "token_sync_kit_interface.h"

using namespace std;
using namespace OHOS::Security::AccessToken;
namespace {
class TokenSyncCallback : public TokenSyncKitInterface {
public:
    ~TokenSyncCallback() = default;
    int32_t GetRemoteHapTokenInfo(const std::string& deviceID, AccessTokenID tokenID) const override
    {
        return TokenSyncError::TOKEN_SYNC_OPENSOURCE_DEVICE; // TOKEN_SYNC_OPENSOURCE_DEVICE is a test
    };

    int32_t DeleteRemoteHapTokenInfo(AccessTokenID tokenID) const override
    {
        return TokenSyncError::TOKEN_SYNC_SUCCESS; // TOKEN_SYNC_SUCCESS is a test
    };

    int32_t UpdateRemoteHapTokenInfo(const HapTokenInfoForSync& tokenInfo) const override
    {
        return TokenSyncError::TOKEN_SYNC_SUCCESS; // TOKEN_SYNC_SUCCESS is a test
    };
};

#ifdef TOKEN_SYNC_ENABLE
static bool NativeTokenGet()
{
    AccessTokenID token = AccessTokenKit::GetNativeTokenId("token_sync_service");
    if (token == 0) {
        return false;
    }
    SetSelfTokenID(token);
    return true;
}
#endif
};

namespace OHOS {
    bool RegisterTokenSyncCallbackFuzzTest(const uint8_t* data, size_t size)
    {
        if ((data == nullptr) || (size == 0)) {
            return false;
        }
    #ifdef TOKEN_SYNC_ENABLE
        std::shared_ptr<TokenSyncKitInterface> callback = std::make_shared<TokenSyncCallback>();
        int32_t result = AccessTokenKit::RegisterTokenSyncCallback(callback);
        return result == RET_SUCCESS;
    #else
        return true;
    #endif // TOKEN_SYNC_ENABLE
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
#ifdef TOKEN_SYNC_ENABLE
    NativeTokenGet();
#endif
    OHOS::RegisterTokenSyncCallbackFuzzTest(data, size);
    return 0;
}