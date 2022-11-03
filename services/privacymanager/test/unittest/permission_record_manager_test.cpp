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

#include "accesstoken_kit.h"
#include "accesstoken_log.h"
#include "constant.h"
#define private public
#include "permission_record_manager.h"
#undef private
#include "privacy_error.h"
#include "privacy_kit.h"
#include "sensitive_resource_manager.h"
#include "state_change_callback.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
namespace Security {
namespace AccessToken {
namespace {
static PermissionStateFull g_testState = {
    .permissionName = "ohos.permission.CAMERA",
    .isGeneral = true,
    .resDeviceID = {"local"},
    .grantStatus = {PermissionState::PERMISSION_GRANTED},
    .grantFlags = {1}
};

static HapPolicyParams g_PolicyPrams1 = {
    .apl = APL_NORMAL,
    .domain = "test.domain.A",
    .permList = {},
    .permStateList = {g_testState}
};

static HapInfoParams g_InfoParms1 = {
    .userID = 1,
    .bundleName = "ohos.privacy_test.bundleA",
    .instIndex = 0,
    .appIDDesc = "privacy_test.bundleA"
};
}
class PermissionRecordManagerTest : public testing::Test {
public:
    static void SetUpTestCase();

    static void TearDownTestCase();

    void SetUp();

    void TearDown();
};

void PermissionRecordManagerTest::SetUpTestCase()
{
}

void PermissionRecordManagerTest::TearDownTestCase()
{
}

void PermissionRecordManagerTest::SetUp()
{
    AccessTokenKit::AllocHapToken(g_InfoParms1, g_PolicyPrams1);
}

void PermissionRecordManagerTest::TearDown()
{
    AccessTokenID tokenId = AccessTokenKit::GetHapTokenID(g_InfoParms1.userID, g_InfoParms1.bundleName,
        g_InfoParms1.instIndex);
    AccessTokenKit::DeleteToken(tokenId);
}

class CbCustomizeTest1 : public StateCustomizedCbk {
public:
    CbCustomizeTest1()
    {}

    ~CbCustomizeTest1()
    {}

    virtual void StateChangeNotify(AccessTokenID tokenId, bool isShow)
    {}

    void Stop()
    {}
};

class CbCustomizeTest2 : public IRemoteObject {
public:
    CbCustomizeTest2()
    {}

    ~CbCustomizeTest2()
    {}
};
/*
 * @tc.name: FindRecordsToUpdateAndExecutedTest001
 * @tc.desc: FindRecordsToUpdateAndExecuted function test with invaild tokenId or permissionName or callback.
 * @tc.type: FUNC
 * @tc.require: issueI5RWX5 issueI5RWX3 issueI5RWXA
 */
HWTEST_F(PermissionRecordManagerTest, FindRecordsToUpdateAndExecutedTest001, TestSize.Level1)
{
    AccessTokenID tokenId = AccessTokenKit::GetHapTokenID(g_InfoParms1.userID, g_InfoParms1.bundleName,
        g_InfoParms1.instIndex);
    ASSERT_NE(0, tokenId);
    ActiveChangeType status = PERM_ACTIVE_IN_BACKGROUND;

    PermissionRecord record1 = {
        .tokenId = tokenId,
        .opCode = Constant::OP_CAMERA,
    };
    PermissionRecordManager::GetInstance().AddRecordToStartList(record1);

    SensitiveResourceManager::GetInstance().SetFlowWindowStatus(tokenId, false);

    std::vector<std::string> permList;
    std::vector<std::string> camPermList;

    PermissionRecordManager::GetInstance().FindRecordsToUpdateAndExecuted(tokenId, status, permList, camPermList);
    PermissionRecordManager::GetInstance().AppStatusListener(tokenId, status);
    
    PermissionRecord record;
    PermissionRecordManager::GetInstance().GetRecordFromStartList(record1.tokenId, record1.opCode, record);
    
    ASSERT_EQ(1, camPermList.size());
    ASSERT_EQ(record1.tokenId, tokenId);
}

/*
 * @tc.name: FindRecordsToUpdateAndExecutedTest002
 * @tc.desc: FindRecordsToUpdateAndExecuted function test with invaild tokenId or permissionName or callback.
 * @tc.type: FUNC
 * @tc.require: issueI5RWX5 issueI5RWX3 issueI5RWXA
 */
HWTEST_F(PermissionRecordManagerTest, FindRecordsToUpdateAndExecutedTest002, TestSize.Level1)
{
    AccessTokenID tokenId = AccessTokenKit::GetHapTokenID(g_InfoParms1.userID, g_InfoParms1.bundleName,
        g_InfoParms1.instIndex);
    ASSERT_NE(0, tokenId);
    ActiveChangeType status = PERM_ACTIVE_IN_BACKGROUND;

    PermissionRecord record1 = {
        .tokenId = tokenId,
        .opCode = Constant::OP_MICROPHONE,
    };
    PermissionRecordManager::GetInstance().AddRecordToStartList(record1);
    SensitiveResourceManager::GetInstance().SetFlowWindowStatus(tokenId, false);

    std::vector<std::string> permList;
    std::vector<std::string> camPermList;
    
    PermissionRecordManager::GetInstance().FindRecordsToUpdateAndExecuted(tokenId, status, permList, camPermList);
    PermissionRecordManager::GetInstance().AppStatusListener(tokenId, status);

    PermissionRecord record;
    PermissionRecordManager::GetInstance().GetRecordFromStartList(record1.tokenId, record1.opCode, record);
    
    ASSERT_EQ(0, camPermList.size());
    ASSERT_EQ(record1.tokenId, tokenId);
}

/*
 * @tc.name: FindRecordsToUpdateAndExecutedTest003
 * @tc.desc: FindRecordsToUpdateAndExecuted function test with invaild tokenId or permissionName or callback.
 * @tc.type: FUNC
 * @tc.require: issueI5RWX5 issueI5RWX3 issueI5RWXA
 */
HWTEST_F(PermissionRecordManagerTest, FindRecordsToUpdateAndExecutedTest003, TestSize.Level1)
{
    AccessTokenID tokenId = AccessTokenKit::GetHapTokenID(g_InfoParms1.userID, g_InfoParms1.bundleName,
        g_InfoParms1.instIndex);
    ASSERT_NE(0, tokenId);
    ActiveChangeType status = PERM_ACTIVE_IN_FOREGROUND;

    PermissionRecord record1 = {
        .tokenId = tokenId,
        .opCode = Constant::OP_CAMERA,
    };
    PermissionRecordManager::GetInstance().AddRecordToStartList(record1);
    SensitiveResourceManager::GetInstance().SetFlowWindowStatus(tokenId, false);

    std::vector<std::string> permList;
    std::vector<std::string> camPermList;
    
    PermissionRecordManager::GetInstance().FindRecordsToUpdateAndExecuted(tokenId, status, permList, camPermList);
    
    PermissionRecord record;
    PermissionRecordManager::GetInstance().GetRecordFromStartList(record1.tokenId, record1.opCode, record);
    
    ASSERT_EQ(0, camPermList.size());
    ASSERT_EQ(record1.tokenId, tokenId);
}

/*
 * @tc.name: FindRecordsToUpdateAndExecutedTest004
 * @tc.desc: FindRecordsToUpdateAndExecuted function test with invaild tokenId or permissionName or callback.
 * @tc.type: FUNC
 * @tc.require: issueI5RWX5 issueI5RWX3 issueI5RWXA
 */
HWTEST_F(PermissionRecordManagerTest, FindRecordsToUpdateAndExecutedTest004, TestSize.Level1)
{
    AccessTokenID tokenId = AccessTokenKit::GetHapTokenID(g_InfoParms1.userID, g_InfoParms1.bundleName,
        g_InfoParms1.instIndex);
    ASSERT_NE(0, tokenId);
    ActiveChangeType status = PERM_ACTIVE_IN_BACKGROUND;
    
    PermissionRecord record1 = {
        .tokenId = tokenId,
        .opCode = Constant::OP_CAMERA,
    };
    PermissionRecordManager::GetInstance().AddRecordToStartList(record1);
    SensitiveResourceManager::GetInstance().SetFlowWindowStatus(tokenId, true);

    std::vector<std::string> permList;
    std::vector<std::string> camPermList;
    
    PermissionRecordManager::GetInstance().FindRecordsToUpdateAndExecuted(tokenId, status, permList, camPermList);
    
    PermissionRecord record;
    PermissionRecordManager::GetInstance().GetRecordFromStartList(record1.tokenId, record1.opCode, record);
    
    ASSERT_EQ(0, camPermList.size());
    ASSERT_EQ(record1.tokenId, tokenId);
}

/*
 * @tc.name: StartUsingPermissionTest001
 * @tc.desc: StartUsingPermission function test with invaild tokenId or permissionName or callback.
 * @tc.type: FUNC
 * @tc.require: issueI5RWX5 issueI5RWX3 issueI5RWXA
 */
HWTEST_F(PermissionRecordManagerTest, StartUsingPermissionTest001, TestSize.Level1)
{
    std::string permissionName = "ohos.permission.CAMERA";
    auto callbackPtr = std::make_shared<CbCustomizeTest1>();
    auto callbackWrap = new (std::nothrow) StateChangeCallback(callbackPtr);
    ASSERT_NE(nullptr, callbackPtr);
    ASSERT_NE(nullptr, callbackWrap);
    AccessTokenID tokenId = AccessTokenKit::GetHapTokenID(g_InfoParms1.userID, g_InfoParms1.bundleName,
        g_InfoParms1.instIndex);
    ASSERT_NE(0, tokenId);
    ASSERT_EQ(ERR_CALLBACK_NOT_EXIST,
        PermissionRecordManager::GetInstance().StartUsingPermission(tokenId, permissionName, nullptr));
    ASSERT_EQ(ERR_TOKENID_NOT_EXIST,
        PermissionRecordManager::GetInstance().StartUsingPermission(0, permissionName, callbackWrap->AsObject()));
}

/*
 * @tc.name: StartUsingPermissionTest002
 * @tc.desc: StartUsingPermission function test with invaild tokenId or permissionName or callback.
 * @tc.type: FUNC
 * @tc.require: issueI5RWXF
 */
HWTEST_F(PermissionRecordManagerTest, StartUsingPermissionTest002, TestSize.Level1)
{
    std::string permissionName = "ohos.permission.CAMERA";
    auto callbackPtr = std::make_shared<CbCustomizeTest1>();
    auto callbackWrap = new (std::nothrow) StateChangeCallback(callbackPtr);
    ASSERT_NE(nullptr, callbackPtr);
    ASSERT_NE(nullptr, callbackWrap);
    AccessTokenID tokenId = AccessTokenKit::GetHapTokenID(g_InfoParms1.userID, g_InfoParms1.bundleName,
        g_InfoParms1.instIndex);
    ASSERT_NE(0, tokenId);
    ASSERT_EQ(ERR_PERMISSION_DENIED, PermissionRecordManager::GetInstance().StartUsingPermission(
        tokenId, "ohos.permission.LOCATION", callbackWrap->AsObject()));
}

/*
 * @tc.name: GetRecordsTest001
 * @tc.desc: Verify the GetRecords abnormal branch function test with.
 * @tc.type: FUNC
 * @tc.require: issueI5RWX5 issueI5RWX3 issueI5RWXA
 */
HWTEST_F(PermissionRecordManagerTest, GetRecordsTest001, TestSize.Level1)
{
    AccessTokenID tokenId = AccessTokenKit::GetHapTokenID(g_InfoParms1.userID, g_InfoParms1.bundleName,
        g_InfoParms1.instIndex);
    ASSERT_NE(0, tokenId);

    std::string permissionName1 = "ohos.permission.CAMERA";
    std::string permissionName2 = "ohos.permission.MICROPHONE";
    
    PermissionRecord record1 = {
        .tokenId = tokenId,
        .opCode = Constant::OP_CAMERA,
    };

    PermissionRecord record2 = {
        .tokenId = tokenId,
        .opCode = Constant::OP_MICROPHONE,
    };

    PermissionRecordManager::GetInstance().AddRecordToStartList(record1);
    PermissionRecordManager::GetInstance().AddRecordToStartList(record2);
    
    PermissionRecordManager::GetInstance().GetRecords(permissionName1, false);
    PermissionRecordManager::GetInstance().GetRecords(permissionName2, false);

    PermissionRecord record3;
    PermissionRecord record4;
    PermissionRecordManager::GetInstance().GetRecordFromStartList(record1.tokenId, record1.opCode, record3);
    PermissionRecordManager::GetInstance().GetRecordFromStartList(record2.tokenId, record2.opCode, record4);

    ASSERT_EQ(record3.tokenId, tokenId);
    ASSERT_EQ(record4.tokenId, tokenId);
}

/*
 * @tc.name: ExecuteCameraCallbackAsyncTest001
 * @tc.desc: Verify the ExecuteCameraCallbackAsync abnormal branch function test with.
 * @tc.type: FUNC
 * @tc.require: issueI5RWX5 issueI5RWX3 issueI5RWXA
 */
HWTEST_F(PermissionRecordManagerTest, ExecuteCameraCallbackAsyncTest001, TestSize.Level1)
{
    AccessTokenID tokenId = AccessTokenKit::GetHapTokenID(g_InfoParms1.userID, g_InfoParms1.bundleName,
        g_InfoParms1.instIndex);
    ASSERT_NE(0, tokenId);

    auto callbackPtr = std::make_shared<CbCustomizeTest1>();
    auto callbackWrap = new (std::nothrow) StateChangeCallback(callbackPtr);
    ASSERT_NE(nullptr, callbackPtr);
    ASSERT_NE(nullptr, callbackWrap);
    PermissionRecordManager::GetInstance().SetCameraCallback(nullptr);
    PermissionRecordManager::GetInstance().ExecuteCameraCallbackAsync(tokenId);

    PermissionRecordManager::GetInstance().SetCameraCallback(callbackWrap->AsObject());
    PermissionRecordManager::GetInstance().ExecuteCameraCallbackAsync(tokenId);
}
} // namespace AccessToken
} // namespace Security
} // namespace OHOS