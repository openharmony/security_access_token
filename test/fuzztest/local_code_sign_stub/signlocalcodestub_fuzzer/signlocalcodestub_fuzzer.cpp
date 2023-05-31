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

#include "signlocalcodestub_fuzzer.h"

#include <cstdint>
#include <string>

#include "accesstoken_kit.h"
#include "access_token.h"
#include "local_code_sign_interface.h"
#include "local_code_sign_service.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

using namespace OHOS::Security::CodeSign;
using namespace OHOS::Security::AccessToken;

namespace OHOS {
    static uint64_t NativeTokenSet(const char *caller)
    {
        uint64_t tokenId = GetSelfTokenID();
        uint64_t mockTokenId = AccessTokenKit::GetNativeTokenId(caller);
        SetSelfTokenID(mockTokenId);
        return tokenId;
    }
    static void NativeTokenReset(uint64_t tokenId)
    {
        SetSelfTokenID(tokenId);
    }


    bool SignLocalCodeStubFuzzTest(const uint8_t *data, size_t size)
    {
        if ((data == nullptr) || (size == 0)) {
            return false;
        }
        std::string testName(reinterpret_cast<const char *>(data), size);
        MessageParcel datas;
        datas.WriteInterfaceToken(ILocalCodeSign::GetDescriptor());
        if (!datas.WriteString(testName)) {
            return false;
        }

        uint32_t code = static_cast<uint32_t>(ILocalCodeSign::SIGN_LOCAL_CODE);
        MessageParcel reply;
        MessageOption option;
        uint64_t selfTokenId = NativeTokenSet("installs");
        DelayedSingleton<LocalCodeSignService>::GetInstance()->OnStart();
        DelayedSingleton<LocalCodeSignService>::GetInstance()->OnRemoteRequest(code, datas, reply, option);
        NativeTokenReset(selfTokenId);
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::SignLocalCodeStubFuzzTest(data, size);
    return 0;
}