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

#include "alloclocaltokenid_fuzzer.h"

#include <string>
#include <vector>
#include <thread>
#include "accesstoken_fuzzdata.h"
#undef private
#include "accesstoken_kit.h"

using namespace std;
using namespace OHOS::Security::AccessToken;

namespace OHOS {
    bool AllocLocalTokenIDFuzzTest(const uint8_t* data, size_t size)
    {
        AccessTokenID tokenId = 0;
        if ((data == nullptr) || (size == 0)) {
            return false;
        }
        AccessTokenFuzzData fuzzData(data, size);

        tokenId = AccessTokenKit::AllocLocalTokenID(fuzzData.GenerateStochasticString(),
            fuzzData.GetData<AccessTokenID>());

        return tokenId != 0;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::AllocLocalTokenIDFuzzTest(data, size);
    return 0;
}
