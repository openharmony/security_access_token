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

#include "unregisterpermactivestatuscallback_fuzzer.h"

#include <iostream>
#include <thread>
#include <string>
#include <vector>

#include "fuzzer/FuzzedDataProvider.h"
#undef private
#include "privacy_kit.h"

using namespace std;
using namespace OHOS::Security::AccessToken;

class UnRegisterActiveFuzzTest : public PermActiveStatusCustomizedCbk {
public:
    explicit UnRegisterActiveFuzzTest(const std::vector<std::string> &permList)
        : PermActiveStatusCustomizedCbk(permList)
    {}

    ~UnRegisterActiveFuzzTest()
    {}

    virtual void ActiveStatusChangeCallback(ActiveChangeResponse& result)
    {
        type_ = result.type;
        return;
    }

    ActiveChangeType type_ = PERM_INACTIVE;
};

namespace OHOS {
    bool UnRegisterPermActiveStatusCallbackFuzzTest(const uint8_t* data, size_t size)
    {
        if ((data == nullptr) || (size == 0)) {
            return false;
        }

        FuzzedDataProvider provider(data, size);
        std::vector<std::string> permList = {provider.ConsumeRandomLengthString()};
        auto callback = std::make_shared<UnRegisterActiveFuzzTest>(permList);

        return PrivacyKit::UnRegisterPermActiveStatusCallback(callback) == 0;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::UnRegisterPermActiveStatusCallbackFuzzTest(data, size);
    return 0;
}
