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

#include <gtest/gtest.h>

#include "access_token.h"
#include "accesstoken_kit.h"
#include "app_manager_access_client.h"
#include "app_manager_access_proxy.h"
#include "app_state_data.h"
#define private public
#include "audio_manager_adapter.h"
#undef private
#ifdef AUDIO_FRAMEWORK_ENABLE
#include "audio_policy_ipc_interface_code.h"
#endif
#include "camera_manager_adapter.h"
#include "permission_record_manager.h"
#include "token_setproc.h"

using namespace testing::ext;

namespace OHOS {
namespace Security {
namespace AccessToken {
class SensitiveManagerServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};
static AccessTokenID g_selfTokenId = 0;
static PermissionStateFull g_testState1 = {
    .permissionName = "ohos.permission.RUNNING_STATE_OBSERVER",
    .isGeneral = true,
    .resDeviceID = {"local"},
    .grantStatus = {PermissionState::PERMISSION_GRANTED},
    .grantFlags = {1}
};

static PermissionStateFull g_testState2 = {
    .permissionName = "ohos.permission.MANAGE_CAMERA_CONFIG",
    .isGeneral = true,
    .resDeviceID = {"local"},
    .grantStatus = {PermissionState::PERMISSION_GRANTED},
    .grantFlags = {1}
};

static PermissionStateFull g_testState3 = {
    .permissionName = "ohos.permission.GET_RUNNING_INFO",
    .isGeneral = true,
    .resDeviceID = {"local"},
    .grantStatus = {PermissionState::PERMISSION_GRANTED},
    .grantFlags = {1}
};

static PermissionStateFull g_testState4 = {
    .permissionName = "ohos.permission.MANAGE_AUDIO_CONFIG",
    .isGeneral = true,
    .resDeviceID = {"local"},
    .grantStatus = {PermissionState::PERMISSION_GRANTED},
    .grantFlags = {1}
};

static PermissionStateFull g_testState5 = {
    .permissionName = "ohos.permission.MICROPHONE_CONTROL",
    .isGeneral = true,
    .resDeviceID = {"local"},
    .grantStatus = {PermissionState::PERMISSION_GRANTED},
    .grantFlags = {1}
};

static HapPolicyParams g_PolicyPrams1 = {
    .apl = APL_NORMAL,
    .domain = "test.domain.A",
    .permList = {},
    .permStateList = {g_testState1, g_testState2, g_testState3, g_testState4, g_testState5}
};

static HapInfoParams g_InfoParms1 = {
    .userID = 1,
    .bundleName = "ohos.privacy_test.bundleA",
    .instIndex = 0,
    .appIDDesc = "privacy_test.bundleA"
};

static HapInfoParams g_infoManagerTestSystemInfoParms = {
    .userID = 1,
    .bundleName = "ohos.privacy_test.bundleB",
    .instIndex = 0,
    .appIDDesc = "privacy_test.bundleB",
    .isSystemApp = true
};
void SensitiveManagerServiceTest::SetUpTestCase()
{
    g_selfTokenId = GetSelfTokenID();
}

void SensitiveManagerServiceTest::TearDownTestCase()
{
}

void SensitiveManagerServiceTest::SetUp()
{
    AccessTokenKit::AllocHapToken(g_InfoParms1, g_PolicyPrams1);
    AccessTokenID tokenId = AccessTokenKit::GetHapTokenID(g_InfoParms1.userID,
                                                          g_InfoParms1.bundleName,
                                                          g_InfoParms1.instIndex);
    EXPECT_EQ(0, SetSelfTokenID(tokenId));
}

void SensitiveManagerServiceTest::TearDown()
{
    AccessTokenID tokenID = AccessTokenKit::GetHapTokenID(g_InfoParms1.userID,
                                                          g_InfoParms1.bundleName,
                                                          g_InfoParms1.instIndex);
    AccessTokenKit::DeleteToken(tokenID);
    EXPECT_EQ(0, SetSelfTokenID(g_selfTokenId));
}

/*
 * @tc.name: RegisterAppObserverTest001
 * @tc.desc: test RegisterApplicationStateObserver with Callback is nullptr.
 * @tc.type: FUNC
 * @tc.require: issueI5RWXF
 */
HWTEST_F(SensitiveManagerServiceTest, RegisterAppObserverTest001, TestSize.Level1)
{
    ASSERT_NE(0, AppManagerAccessClient::GetInstance().RegisterApplicationStateObserver(nullptr));
    ASSERT_NE(0, AppManagerAccessClient::GetInstance().UnregisterApplicationStateObserver(nullptr));
}

/*
 * @tc.name: RegisterAppObserverTest002
 * @tc.desc: test RegisterApplicationStateObserver with callback is not nullptr
 * @tc.type: FUNC
 * @tc.require: issueI5RWXF
 */
HWTEST_F(SensitiveManagerServiceTest, RegisterAppObserverTest002, TestSize.Level1)
{
    AccessTokenID tokenId = AccessTokenKit::GetNativeTokenId("privacy_service");
    EXPECT_EQ(0, SetSelfTokenID(tokenId));

    sptr<ApplicationStateObserverStub> listener = new(std::nothrow) ApplicationStateObserverStub();
    ASSERT_NE(listener, nullptr);
    ASSERT_EQ(0, AppManagerAccessClient::GetInstance().RegisterApplicationStateObserver(listener));

    std::vector<AppStateData> list;
    ASSERT_EQ(0, AppManagerAccessClient::GetInstance().GetForegroundApplications(list));
    for (size_t i = 0; i < list.size(); ++i) {
        ASSERT_NE(tokenId, list[i].accessTokenId);
    }

    ASSERT_EQ(0, AppManagerAccessClient::GetInstance().UnregisterApplicationStateObserver(listener));
}
} // namespace AccessToken
} // namespace Security
} // namespace OHOS
