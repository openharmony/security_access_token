/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef ACCESSTOKEN_MANAGER_SERVICE_TEST_H
#define ACCESSTOKEN_MANAGER_SERVICE_TEST_H
#include <gtest/gtest.h>
#define private public
#include "accesstoken_manager_service.h"
#undef private

namespace OHOS {
namespace Security {
namespace AccessToken {
class AccessTokenManagerServiceTest : public testing::Test {
public:
    static void SetUpTestCase();

    static void TearDownTestCase();

    void SetUp();

    void TearDown();
    void CreateHapToken(const HapInfoParcel& infoParCel, const HapPolicyParcel& policyParcel, AccessTokenID& tokenId,
        std::map<int32_t, TokenIdInfo>& tokenIdAplMap, bool hasInit = false);
    void DelTestDataAndRestoreOri(AccessTokenID tokenId, const std::vector<GenericValues>& oriData);

    std::shared_ptr<AccessTokenManagerService> atManagerService_;
};
} // namespace AccessToken
} // namespace Security
} // namespace OHOS
#endif // ACCESSTOKEN_MANAGER_SERVICE_TEST_H