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

#include "app_installation_optimized_test.h"
#include <thread>

#include "access_token_error.h"
#include "accesstoken_common_log.h"
#include "iaccess_token_manager.h"
#include "nativetoken_kit.h"
#include "permission_grant_info.h"
#include "permission_state_change_info_parcel.h"
#include "string_ex.h"
#include "test_common.h"
#include "tokenid_kit.h"
#include "token_setproc.h"

using namespace testing::ext;
namespace OHOS {
namespace Security {
namespace AccessToken {
namespace {
const std::string CERT_PERMISSION = "ohos.permission.ACCESS_CERT_MANAGER";
const std::string CALENDAR_PERMISSION = "ohos.permission.WRITE_CALENDAR";
const std::string APP_TRACKING_PERMISSION = "ohos.permission.APP_TRACKING_CONSENT";
const std::string ACCESS_BLUETOOTH_PERMISSION = "ohos.permission.ACCESS_BLUETOOTH";
static constexpr int32_t DEFAULT_API_VERSION = 8;
static constexpr int32_t MAX_PERM_LIST_SIZE = 1024;
static MockNativeToken* g_mock;

PermissionStateFull g_infoManagerCameraState = {
    .permissionName = APP_TRACKING_PERMISSION,
    .isGeneral = true,
    .resDeviceID = {"local2"},
    .grantStatus = {PermissionState::PERMISSION_DENIED},
    .grantFlags = {0}
};

PermissionStateFull g_infoBlueToothManagerState = {
    .permissionName = ACCESS_BLUETOOTH_PERMISSION,
    .isGeneral = true,
    .resDeviceID = {"local2"},
    .grantStatus = {PermissionState::PERMISSION_DENIED},
    .grantFlags = {0}
};

PermissionStateFull g_infoManagerMicrophoneState = {
    .permissionName = CALENDAR_PERMISSION,
    .isGeneral = true,
    .resDeviceID = {"local2"},
    .grantStatus = {PermissionState::PERMISSION_DENIED},
    .grantFlags = {0}
};

PermissionStateFull g_infoManagerCertState = {
    .permissionName = CERT_PERMISSION,
    .isGeneral = true,
    .resDeviceID = {"local3"},
    .grantStatus = {PermissionState::PERMISSION_DENIED},
    .grantFlags = {0}
};

PermissionStateFull g_infoManagerTestState4 = {
    .permissionName = "ohos.permission.ACCESS_BUNDLE_DIR",
    .isGeneral = true,
    .resDeviceID = {"local3"},
    .grantStatus = {PermissionState::PERMISSION_DENIED},
    .grantFlags = {0}
};

PermissionStateFull g_infoManagerTestStateMdm = {
    .permissionName = "ohos.permission.SET_ENTERPRISE_INFO",
    .isGeneral = true,
    .resDeviceID = {"local3"},
    .grantStatus = {PermissionState::PERMISSION_DENIED},
    .grantFlags = {0}
};

HapInfoParams g_testHapInfoParams = {
    .userID = 1,
    .bundleName = "testName",
    .instIndex = 0,
    .appIDDesc = "test2",
    .apiVersion = 11 // api version is 11
};

HapPolicyParams g_testPolicyParams = {
    .apl = APL_NORMAL,
    .domain = "test.domain2",
    .permStateList = {
        g_infoManagerCameraState,
        g_infoManagerMicrophoneState,
        g_infoManagerCertState
    }
};
uint64_t g_selfShellTokenId;
}

void AppInstallationOptimizedTest::SetUpTestCase()
{
    g_selfShellTokenId = GetSelfTokenID();
    TestCommon::SetTestEvironment(g_selfShellTokenId);
    g_mock = new (std::nothrow) MockNativeToken("foundation");

    AccessTokenIDEx tokenIdEx = TestCommon::GetHapTokenIdFromBundle(
        g_testHapInfoParams.userID, g_testHapInfoParams.bundleName, g_testHapInfoParams.instIndex);
    AccessTokenKit::DeleteToken(tokenIdEx.tokenIdExStruct.tokenID);
}

void AppInstallationOptimizedTest::TearDownTestCase()
{
    if (g_mock != nullptr) {
        delete g_mock;
        g_mock = nullptr;
    }
    AccessTokenIDEx tokenIdEx = TestCommon::GetHapTokenIdFromBundle(
        g_testHapInfoParams.userID, g_testHapInfoParams.bundleName, g_testHapInfoParams.instIndex);
    AccessTokenKit::DeleteToken(tokenIdEx.tokenIdExStruct.tokenID);

    EXPECT_EQ(0, SetSelfTokenID(g_selfShellTokenId));
    TestCommon::ResetTestEvironment();
}

void AppInstallationOptimizedTest::SetUp()
{
    LOGI(ATM_DOMAIN, ATM_TAG, "SetUp ok.");
}

void AppInstallationOptimizedTest::TearDown()
{
}

/**
 * @tc.name: InitHapToken001
 * @tc.desc:Init a tokenId successfully, delete it successfully the first time and fail to delete it again.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(AppInstallationOptimizedTest, InitHapToken001, TestSize.Level0)
{
    AccessTokenIDEx fullTokenId;
    int32_t ret = AccessTokenKit::InitHapToken(g_testHapInfoParams, g_testPolicyParams, fullTokenId);
    EXPECT_EQ(RET_SUCCESS, ret);
    ASSERT_EQ(RET_SUCCESS, AccessTokenKit::DeleteToken(fullTokenId.tokenIdExStruct.tokenID));
    ASSERT_NE(RET_SUCCESS, AccessTokenKit::DeleteToken(fullTokenId.tokenIdExStruct.tokenID));
}

/**
 * @tc.name: InitHapToken002
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(AppInstallationOptimizedTest, InitHapToken002, TestSize.Level0)
{
    HapPolicyParams testPolicyParams = {
        .apl = APL_SYSTEM_BASIC,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerCertState}
    };
    AccessTokenIDEx fullTokenId;
    int32_t res = AccessTokenKit::InitHapToken(g_testHapInfoParams, testPolicyParams, fullTokenId);
    GTEST_LOG_(INFO) << "tokenID :" << fullTokenId.tokenIdExStruct.tokenID;
    EXPECT_EQ(RET_SUCCESS, res);
    int32_t ret = AccessTokenKit::VerifyAccessToken(
        fullTokenId.tokenIdExStruct.tokenID, CERT_PERMISSION);
    EXPECT_EQ(ret, PERMISSION_GRANTED);
    std::vector<PermissionStateFull> permStatList;
    res = TestCommon::GetReqPermissionsByTest(fullTokenId.tokenIdExStruct.tokenID, permStatList, true);
    ASSERT_EQ(RET_SUCCESS, res);
    EXPECT_EQ(static_cast<uint32_t>(1), permStatList.size());
    EXPECT_EQ(CERT_PERMISSION, permStatList[0].permissionName);
    EXPECT_EQ(permStatList[0].grantFlags[0], PERMISSION_SYSTEM_FIXED);
    EXPECT_EQ(RET_SUCCESS, AccessTokenKit::DeleteToken(fullTokenId.tokenIdExStruct.tokenID));
}

/**
 * @tc.name: InitHapToken003
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(AppInstallationOptimizedTest, InitHapToken003, TestSize.Level0)
{
    HapPolicyParams testPolicyParams = {
        .apl = APL_NORMAL,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerMicrophoneState}
    };
    AccessTokenIDEx fullTokenId;
    int32_t res = AccessTokenKit::InitHapToken(g_testHapInfoParams, testPolicyParams, fullTokenId);
    EXPECT_EQ(RET_SUCCESS, res);
    int32_t ret = AccessTokenKit::VerifyAccessToken(
        fullTokenId.tokenIdExStruct.tokenID, CALENDAR_PERMISSION);
    EXPECT_EQ(ret, PERMISSION_DENIED);
    uint32_t flag;
    TestCommon::GetPermissionFlagByTest(fullTokenId.tokenIdExStruct.tokenID, CALENDAR_PERMISSION, flag);
    EXPECT_EQ(flag, PERMISSION_DEFAULT_FLAG);
    EXPECT_EQ(RET_SUCCESS, AccessTokenKit::DeleteToken(fullTokenId.tokenIdExStruct.tokenID));
}

/**
 * @tc.name: InitHapToken004
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(AppInstallationOptimizedTest, InitHapToken004, TestSize.Level0)
{
    PreAuthorizationInfo info1 = {
        .permissionName = CALENDAR_PERMISSION,
        .userCancelable = false
    };
    HapPolicyParams testPolicyParams = {
        .apl = APL_NORMAL,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerMicrophoneState},
        .preAuthorizationInfo = {info1},
    };
    AccessTokenIDEx fullTokenId;
    int32_t res = AccessTokenKit::InitHapToken(g_testHapInfoParams, testPolicyParams, fullTokenId);
    EXPECT_EQ(RET_SUCCESS, res);
    int32_t ret = AccessTokenKit::VerifyAccessToken(
        fullTokenId.tokenIdExStruct.tokenID, CALENDAR_PERMISSION);
    EXPECT_EQ(ret, PERMISSION_GRANTED);
    std::vector<PermissionStateFull> permStatList;
    res = TestCommon::GetReqPermissionsByTest(fullTokenId.tokenIdExStruct.tokenID, permStatList, false);
    EXPECT_EQ(RET_SUCCESS, res);
    EXPECT_EQ(static_cast<uint32_t>(1), permStatList.size());
    EXPECT_EQ(CALENDAR_PERMISSION, permStatList[0].permissionName);
    EXPECT_EQ(permStatList[0].grantFlags[0], PERMISSION_SYSTEM_FIXED);
    EXPECT_EQ(RET_SUCCESS, AccessTokenKit::DeleteToken(fullTokenId.tokenIdExStruct.tokenID));
}


/**
 * @tc.name: InitHapToken005
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(AppInstallationOptimizedTest, InitHapToken005, TestSize.Level0)
{
    PreAuthorizationInfo info1 = {
        .permissionName = CALENDAR_PERMISSION,
        .userCancelable = true
    };
    HapPolicyParams testPolicyParams = {
        .apl = APL_NORMAL,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerMicrophoneState},
        .preAuthorizationInfo = {info1},
    };
    AccessTokenIDEx fullTokenId;
    int32_t res = AccessTokenKit::InitHapToken(g_testHapInfoParams, testPolicyParams, fullTokenId);
    EXPECT_EQ(RET_SUCCESS, res);
    int32_t ret = AccessTokenKit::VerifyAccessToken(
        fullTokenId.tokenIdExStruct.tokenID, CALENDAR_PERMISSION);
    EXPECT_EQ(ret, PERMISSION_GRANTED);

    std::vector<PermissionStateFull> permStatList;
    res = TestCommon::GetReqPermissionsByTest(fullTokenId.tokenIdExStruct.tokenID, permStatList, false);
    EXPECT_EQ(RET_SUCCESS, res);
    EXPECT_EQ(static_cast<uint32_t>(1), permStatList.size());
    EXPECT_EQ(CALENDAR_PERMISSION, permStatList[0].permissionName);
    EXPECT_EQ(permStatList[0].grantFlags[0], PERMISSION_PRE_AUTHORIZED_CANCELABLE);
    EXPECT_EQ(RET_SUCCESS, AccessTokenKit::DeleteToken(fullTokenId.tokenIdExStruct.tokenID));
}

/**
 * @tc.name: InitHapToken006
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(AppInstallationOptimizedTest, InitHapToken006, TestSize.Level0)
{
    HapPolicyParams testPolicyParam = {
        .apl = APL_SYSTEM_BASIC,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerTestStateMdm},
    };
    g_testHapInfoParams.appDistributionType = "enterprise_mdm";
    AccessTokenIDEx fullTokenId;
    int32_t res = AccessTokenKit::InitHapToken(g_testHapInfoParams, testPolicyParam, fullTokenId);
    EXPECT_EQ(RET_SUCCESS, res);
    EXPECT_EQ(RET_SUCCESS, AccessTokenKit::DeleteToken(fullTokenId.tokenIdExStruct.tokenID));
}

/**
 * @tc.name: InitHapToken007
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(AppInstallationOptimizedTest, InitHapToken007, TestSize.Level0)
{
    HapPolicyParams testPolicyParam = {
        .apl = APL_SYSTEM_BASIC,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerTestStateMdm},
    };
    g_testHapInfoParams.appDistributionType = "";
    AccessTokenIDEx fullTokenId;
    int32_t res = AccessTokenKit::InitHapToken(g_testHapInfoParams, testPolicyParam, fullTokenId);
    EXPECT_EQ(ERR_PERM_REQUEST_CFG_FAILED, res);
}

/**
 * @tc.name: InitHapToken008
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(AppInstallationOptimizedTest, InitHapToken008, TestSize.Level0)
{
    HapPolicyParams testPolicyParam = {
        .apl = APL_SYSTEM_BASIC,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerTestStateMdm},
    };
    g_testHapInfoParams.appDistributionType = "none";
    AccessTokenIDEx fullTokenId;
    int32_t res = AccessTokenKit::InitHapToken(g_testHapInfoParams, testPolicyParam, fullTokenId);
    EXPECT_EQ(RET_SUCCESS, res);
}

/**
 * @tc.name: InitHapToken009
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(AppInstallationOptimizedTest, InitHapToken009, TestSize.Level0)
{
    HapPolicyParams testPolicyParam = {
        .apl = APL_NORMAL,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerTestState4},
    };

    AccessTokenIDEx fullTokenId;
    int32_t res = AccessTokenKit::InitHapToken(g_testHapInfoParams, testPolicyParam, fullTokenId);
    EXPECT_EQ(ERR_PERM_REQUEST_CFG_FAILED, res);
}

/**
 * @tc.name: InitHapToken010
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(AppInstallationOptimizedTest, InitHapToken010, TestSize.Level0)
{
    HapPolicyParams testPolicyParam = {
        .apl = APL_SYSTEM_BASIC,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerTestState4},
    };
    g_testHapInfoParams.appDistributionType = "";
    AccessTokenIDEx fullTokenId;
    int32_t res = AccessTokenKit::InitHapToken(g_testHapInfoParams, testPolicyParam, fullTokenId);
    EXPECT_EQ(RET_SUCCESS, res);
    EXPECT_EQ(RET_SUCCESS, AccessTokenKit::DeleteToken(fullTokenId.tokenIdExStruct.tokenID));
}

#ifdef SUPPORT_SANDBOX_APP
/**
 * @tc.name: InitHapToken011
 * @tc.desc: InitHapToken with dlp type.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(AppInstallationOptimizedTest, InitHapToken011, TestSize.Level0)
{
    HapPolicyParams testPolicyParams = {
        .apl = APL_SYSTEM_BASIC,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerCameraState, g_infoManagerMicrophoneState, g_infoManagerCertState}
    };
    AccessTokenIDEx fullTokenId;
    int32_t res = AccessTokenKit::InitHapToken(g_testHapInfoParams, testPolicyParams, fullTokenId);
    EXPECT_EQ(RET_SUCCESS, res);

    HapInfoParams testHapInfoParams1 = g_testHapInfoParams;
    testHapInfoParams1.dlpType = DLP_FULL_CONTROL;
    testHapInfoParams1.instIndex++;
    AccessTokenIDEx dlpFullTokenId1;
    res = AccessTokenKit::InitHapToken(testHapInfoParams1, testPolicyParams, dlpFullTokenId1);
    EXPECT_EQ(RET_SUCCESS, res);

    res = AccessTokenKit::VerifyAccessToken(dlpFullTokenId1.tokenIdExStruct.tokenID, APP_TRACKING_PERMISSION);
    EXPECT_EQ(res, PERMISSION_DENIED);

    (void)TestCommon::GrantPermissionByTest(
        fullTokenId.tokenIdExStruct.tokenID, APP_TRACKING_PERMISSION, PERMISSION_USER_SET);
    (void)TestCommon::RevokePermissionByTest(
        fullTokenId.tokenIdExStruct.tokenID, CALENDAR_PERMISSION, PERMISSION_USER_SET);

    testHapInfoParams1.instIndex++;
    AccessTokenIDEx dlpFullTokenId2;
    res = AccessTokenKit::InitHapToken(testHapInfoParams1, testPolicyParams, dlpFullTokenId2);
    EXPECT_EQ(RET_SUCCESS, res);
    res = AccessTokenKit::VerifyAccessToken(dlpFullTokenId2.tokenIdExStruct.tokenID, APP_TRACKING_PERMISSION);
    EXPECT_EQ(res, PERMISSION_GRANTED);
    res = AccessTokenKit::VerifyAccessToken(dlpFullTokenId1.tokenIdExStruct.tokenID, APP_TRACKING_PERMISSION);
    EXPECT_EQ(res, PERMISSION_GRANTED);

    std::vector<PermissionStateFull> permStatList1;
    res = TestCommon::GetReqPermissionsByTest(fullTokenId.tokenIdExStruct.tokenID, permStatList1, false);
    ASSERT_EQ(RET_SUCCESS, res);
    std::vector<PermissionStateFull> permStatList2;
    res = TestCommon::GetReqPermissionsByTest(dlpFullTokenId2.tokenIdExStruct.tokenID, permStatList2, false);
    EXPECT_EQ(permStatList2.size(), permStatList1.size());
    EXPECT_EQ(APP_TRACKING_PERMISSION, permStatList2[0].permissionName);
    EXPECT_EQ(permStatList2[0].grantStatus[0], PERMISSION_GRANTED);
    EXPECT_EQ(permStatList2[0].grantFlags[0], PERMISSION_USER_SET);
    EXPECT_EQ(CALENDAR_PERMISSION, permStatList2[1].permissionName);
    EXPECT_EQ(permStatList2[1].grantStatus[0], PERMISSION_DENIED);
    EXPECT_EQ(permStatList2[1].grantFlags[0], PERMISSION_USER_SET);
    EXPECT_EQ(RET_SUCCESS, res);
}
#endif

/**
 * @tc.name: UpdateHapToken001
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(AppInstallationOptimizedTest, UpdateHapToken001, TestSize.Level0)
{
    HapPolicyParams testPolicyParams1 = {
        .apl = APL_NORMAL,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerCameraState, g_infoBlueToothManagerState, g_infoManagerCertState}
    };
    AccessTokenIDEx fullTokenId;
    int32_t ret = AccessTokenKit::InitHapToken(g_testHapInfoParams, testPolicyParams1, fullTokenId);
    GTEST_LOG_(INFO) << "tokenID :" << fullTokenId.tokenIdExStruct.tokenID;
    EXPECT_EQ(RET_SUCCESS, ret);
    ret = AccessTokenKit::VerifyAccessToken(
        fullTokenId.tokenIdExStruct.tokenID, APP_TRACKING_PERMISSION);
    EXPECT_EQ(PERMISSION_DENIED, ret);
    ret = AccessTokenKit::VerifyAccessToken(
        fullTokenId.tokenIdExStruct.tokenID, CERT_PERMISSION);
    EXPECT_EQ(PERMISSION_GRANTED, ret);

    ret = TestCommon::GrantPermissionByTest(fullTokenId.tokenIdExStruct.tokenID, APP_TRACKING_PERMISSION, 0);
    EXPECT_EQ(RET_SUCCESS, ret);
    ret = AccessTokenKit::VerifyAccessToken(fullTokenId.tokenIdExStruct.tokenID, APP_TRACKING_PERMISSION);
    EXPECT_EQ(PERMISSION_GRANTED, ret);
    ret = TestCommon::GrantPermissionByTest(
    fullTokenId.tokenIdExStruct.tokenID, ACCESS_BLUETOOTH_PERMISSION, PERMISSION_SYSTEM_FIXED);
    EXPECT_EQ(RET_SUCCESS, ret);
    ret = AccessTokenKit::VerifyAccessToken(
        fullTokenId.tokenIdExStruct.tokenID, ACCESS_BLUETOOTH_PERMISSION);
    EXPECT_EQ(PERMISSION_GRANTED, ret);

    UpdateHapInfoParams info;
    info.appIDDesc = "TEST";
    info.apiVersion = DEFAULT_API_VERSION;
    info.isSystemApp = false;
    HapPolicyParams testPolicyParams2 = {
        .apl = APL_NORMAL,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerCameraState, g_infoBlueToothManagerState, g_infoManagerMicrophoneState}
    };
    ret = AccessTokenKit::UpdateHapToken(fullTokenId, info, testPolicyParams2);
    ASSERT_EQ(RET_SUCCESS, ret);
    ret = AccessTokenKit::VerifyAccessToken(fullTokenId.tokenIdExStruct.tokenID, APP_TRACKING_PERMISSION);
    EXPECT_EQ(PERMISSION_GRANTED, ret);
    ret = AccessTokenKit::VerifyAccessToken(fullTokenId.tokenIdExStruct.tokenID, CALENDAR_PERMISSION);
    EXPECT_EQ(PERMISSION_DENIED, ret);
    ret = AccessTokenKit::VerifyAccessToken(fullTokenId.tokenIdExStruct.tokenID, ACCESS_BLUETOOTH_PERMISSION);
    EXPECT_EQ(PERMISSION_DENIED, ret);
    ret = AccessTokenKit::VerifyAccessToken(fullTokenId.tokenIdExStruct.tokenID, CERT_PERMISSION);
    EXPECT_EQ(PERMISSION_DENIED, ret);

    EXPECT_EQ(RET_SUCCESS, AccessTokenKit::DeleteToken(fullTokenId.tokenIdExStruct.tokenID));
}

/**
 * @tc.name: UpdateHapToken002
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(AppInstallationOptimizedTest, UpdateHapToken002, TestSize.Level0)
{
    HapPolicyParams testPolicyParams1 = {
        .apl = APL_NORMAL,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerCameraState}
    };
    AccessTokenIDEx fullTokenId;
    int32_t ret = AccessTokenKit::InitHapToken(g_testHapInfoParams, testPolicyParams1, fullTokenId);
    EXPECT_EQ(RET_SUCCESS, ret);
    ret = TestCommon::RevokePermissionByTest(
        fullTokenId.tokenIdExStruct.tokenID, APP_TRACKING_PERMISSION, PERMISSION_USER_FIXED);
    EXPECT_EQ(RET_SUCCESS, ret);

    std::vector<PermissionStateFull> permStatList;
    int32_t res = TestCommon::GetReqPermissionsByTest(fullTokenId.tokenIdExStruct.tokenID, permStatList, false);
    ASSERT_EQ(RET_SUCCESS, res);
    EXPECT_EQ(static_cast<uint32_t>(1), permStatList.size());
    EXPECT_EQ(APP_TRACKING_PERMISSION, permStatList[0].permissionName);
    EXPECT_EQ(permStatList[0].grantStatus[0], PERMISSION_DENIED);
    EXPECT_EQ(permStatList[0].grantFlags[0], PERMISSION_USER_FIXED);

    UpdateHapInfoParams info;
    info.appIDDesc = "TEST";
    info.apiVersion = DEFAULT_API_VERSION;
    info.isSystemApp = false;
    HapPolicyParams testPolicyParams2 = {
        .apl = APL_NORMAL,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerCameraState, g_infoManagerMicrophoneState}
    };
    ret = AccessTokenKit::UpdateHapToken(fullTokenId, info, testPolicyParams2);
    ASSERT_EQ(RET_SUCCESS, ret);
    std::vector<PermissionStateFull> permStatList1;
    res = TestCommon::GetReqPermissionsByTest(fullTokenId.tokenIdExStruct.tokenID, permStatList1, false);
    ASSERT_EQ(RET_SUCCESS, res);
    EXPECT_EQ(static_cast<uint32_t>(2), permStatList1.size());
    EXPECT_EQ(APP_TRACKING_PERMISSION, permStatList1[0].permissionName);
    EXPECT_EQ(permStatList1[0].grantStatus[0], PERMISSION_DENIED);
    EXPECT_EQ(permStatList1[0].grantFlags[0], PERMISSION_USER_FIXED);
    EXPECT_EQ(CALENDAR_PERMISSION, permStatList1[1].permissionName);
    EXPECT_EQ(permStatList1[1].grantStatus[0], PERMISSION_DENIED);
    EXPECT_EQ(permStatList1[1].grantFlags[0], PERMISSION_DEFAULT_FLAG);

    EXPECT_EQ(RET_SUCCESS, AccessTokenKit::DeleteToken(fullTokenId.tokenIdExStruct.tokenID));
}

/**
 * @tc.name: UpdateHapToken003
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(AppInstallationOptimizedTest, UpdateHapToken003, TestSize.Level0)
{
    HapPolicyParams testPolicyParams1 = {
        .apl = APL_NORMAL,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerCameraState}
    };
    AccessTokenIDEx fullTokenId;
    int32_t ret = AccessTokenKit::InitHapToken(g_testHapInfoParams, testPolicyParams1, fullTokenId);
    GTEST_LOG_(INFO) << "tokenID :" << fullTokenId.tokenIdExStruct.tokenID;
    EXPECT_EQ(RET_SUCCESS, ret);
    ret = AccessTokenKit::VerifyAccessToken(
        fullTokenId.tokenIdExStruct.tokenID, APP_TRACKING_PERMISSION);
    EXPECT_EQ(PERMISSION_DENIED, ret);

    UpdateHapInfoParams info;
    info.appIDDesc = "TEST";
    info.apiVersion = DEFAULT_API_VERSION;
    info.isSystemApp = false;
    PreAuthorizationInfo info1 = {
        .permissionName = CALENDAR_PERMISSION,
        .userCancelable = true
    };
    HapPolicyParams testPolicyParams2 = {
        .apl = APL_NORMAL,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerCameraState, g_infoManagerMicrophoneState},
        .preAuthorizationInfo = {info1}
    };
    ret = AccessTokenKit::UpdateHapToken(fullTokenId, info, testPolicyParams2);
    ASSERT_EQ(RET_SUCCESS, ret);
    std::vector<PermissionStateFull> permStatList1;
    int32_t res = TestCommon::GetReqPermissionsByTest(fullTokenId.tokenIdExStruct.tokenID, permStatList1, false);
    ASSERT_EQ(RET_SUCCESS, res);
    EXPECT_EQ(static_cast<uint32_t>(2), permStatList1.size());
    EXPECT_EQ(APP_TRACKING_PERMISSION, permStatList1[0].permissionName);
    EXPECT_EQ(permStatList1[0].grantStatus[0], PERMISSION_DENIED);
    EXPECT_EQ(permStatList1[0].grantFlags[0], PERMISSION_DEFAULT_FLAG);
    EXPECT_EQ(CALENDAR_PERMISSION, permStatList1[1].permissionName);
    EXPECT_EQ(permStatList1[1].grantStatus[0], PERMISSION_GRANTED);
    EXPECT_EQ(permStatList1[1].grantFlags[0], PERMISSION_PRE_AUTHORIZED_CANCELABLE);

    EXPECT_EQ(RET_SUCCESS, AccessTokenKit::DeleteToken(fullTokenId.tokenIdExStruct.tokenID));
}

/**
 * @tc.name: UpdateHapToken004
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(AppInstallationOptimizedTest, UpdateHapToken004, TestSize.Level0)
{
    HapPolicyParams testPolicyParams1 = {
        .apl = APL_NORMAL,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerCameraState}
    };
    AccessTokenIDEx fullTokenId;
    int32_t ret = AccessTokenKit::InitHapToken(g_testHapInfoParams, testPolicyParams1, fullTokenId);
    GTEST_LOG_(INFO) << "tokenID :" << fullTokenId.tokenIdExStruct.tokenID;
    EXPECT_EQ(RET_SUCCESS, ret);

    UpdateHapInfoParams info;
    info.appIDDesc = "TEST";
    info.apiVersion = DEFAULT_API_VERSION;
    info.isSystemApp = true;
    PreAuthorizationInfo info1 = {
        .permissionName = CALENDAR_PERMISSION,
        .userCancelable = false
    };
    HapPolicyParams testPolicyParams2 = {
        .apl = APL_NORMAL,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerCameraState, g_infoManagerMicrophoneState},
        .preAuthorizationInfo = {info1}
    };
    ret = AccessTokenKit::UpdateHapToken(fullTokenId, info, testPolicyParams2);
    ASSERT_EQ(RET_SUCCESS, ret);
    std::vector<PermissionStateFull> state;
    int32_t res = TestCommon::GetReqPermissionsByTest(fullTokenId.tokenIdExStruct.tokenID, state, false);
    ASSERT_EQ(RET_SUCCESS, res);
    EXPECT_EQ(static_cast<uint32_t>(2), state.size());
    EXPECT_EQ(CALENDAR_PERMISSION, state[1].permissionName);
    EXPECT_EQ(state[1].grantStatus[0], PERMISSION_GRANTED);
    EXPECT_EQ(state[1].grantFlags[0], PERMISSION_SYSTEM_FIXED);

    EXPECT_EQ(RET_SUCCESS, AccessTokenKit::DeleteToken(fullTokenId.tokenIdExStruct.tokenID));
}

/**
 * @tc.name: UpdateHapToken005
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(AppInstallationOptimizedTest, UpdateHapToken005, TestSize.Level0)
{
    HapPolicyParams testPolicyParam = {
        .apl = APL_SYSTEM_BASIC,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerCameraState}
    };
    AccessTokenIDEx fullTokenId;
    int32_t res = AccessTokenKit::InitHapToken(g_testHapInfoParams, testPolicyParam, fullTokenId);
    EXPECT_EQ(RET_SUCCESS, res);

    HapPolicyParams testPolicyParam2 = {
        .apl = APL_SYSTEM_BASIC,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerCameraState, g_infoManagerTestStateMdm},
    };
    UpdateHapInfoParams info = {
        .appIDDesc = "TEST",
        .apiVersion = DEFAULT_API_VERSION,
        .isSystemApp = false
    };
    info.appDistributionType = "enterprise_mdm";
    res = AccessTokenKit::UpdateHapToken(fullTokenId, info, testPolicyParam2);
    EXPECT_EQ(RET_SUCCESS, res);
    EXPECT_EQ(RET_SUCCESS, AccessTokenKit::DeleteToken(fullTokenId.tokenIdExStruct.tokenID));
}

/**
 * @tc.name: UpdateHapToken006
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(AppInstallationOptimizedTest, UpdateHapToken006, TestSize.Level0)
{
    HapPolicyParams testPolicyParam = {
        .apl = APL_SYSTEM_BASIC,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerCameraState}
    };
    AccessTokenIDEx fullTokenId;
    int32_t res = AccessTokenKit::InitHapToken(g_testHapInfoParams, testPolicyParam, fullTokenId);
    EXPECT_EQ(RET_SUCCESS, res);

    HapPolicyParams testPolicyParam2 = {
        .apl = APL_SYSTEM_BASIC,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerCameraState, g_infoManagerTestStateMdm},
    };
    UpdateHapInfoParams info = {
        .appIDDesc = "TEST",
        .apiVersion = DEFAULT_API_VERSION,
        .isSystemApp = false
    };
    info.appDistributionType = "";
    res = AccessTokenKit::UpdateHapToken(fullTokenId, info, testPolicyParam2);
    EXPECT_EQ(ERR_PERM_REQUEST_CFG_FAILED, res);
    EXPECT_EQ(RET_SUCCESS, AccessTokenKit::DeleteToken(fullTokenId.tokenIdExStruct.tokenID));
}

/**
 * @tc.name: UpdateHapToken007
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(AppInstallationOptimizedTest, UpdateHapToken007, TestSize.Level0)
{
    HapPolicyParams testPolicyParam = {
        .apl = APL_SYSTEM_BASIC,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerCameraState}
    };
    AccessTokenIDEx fullTokenId;
    int32_t res = AccessTokenKit::InitHapToken(g_testHapInfoParams, testPolicyParam, fullTokenId);
    EXPECT_EQ(RET_SUCCESS, res);

    HapPolicyParams testPolicyParam2 = {
        .apl = APL_SYSTEM_BASIC,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerCameraState, g_infoManagerTestStateMdm},
    };
    UpdateHapInfoParams info = {
        .appIDDesc = "TEST",
        .apiVersion = DEFAULT_API_VERSION,
        .isSystemApp = false
    };
    info.appDistributionType = "none";
    res = AccessTokenKit::UpdateHapToken(fullTokenId, info, testPolicyParam2);
    EXPECT_EQ(RET_SUCCESS, res);
    EXPECT_EQ(RET_SUCCESS, AccessTokenKit::DeleteToken(fullTokenId.tokenIdExStruct.tokenID));
}

/**
 * @tc.name: UpdateHapToken008
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(AppInstallationOptimizedTest, UpdateHapToken008, TestSize.Level0)
{
    HapPolicyParams testPolicyParam = {
        .apl = APL_NORMAL,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerCameraState}
    };
    AccessTokenIDEx fullTokenId;
    int32_t res = AccessTokenKit::InitHapToken(g_testHapInfoParams, testPolicyParam, fullTokenId);
    EXPECT_EQ(RET_SUCCESS, res);

    HapPolicyParams testPolicyParam2 = {
        .apl = APL_SYSTEM_BASIC,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerCameraState, g_infoManagerTestState4},
    };
    UpdateHapInfoParams info = {
        .appIDDesc = "TEST",
        .apiVersion = DEFAULT_API_VERSION,
        .isSystemApp = true
    };
    res = AccessTokenKit::UpdateHapToken(fullTokenId, info, testPolicyParam2);
    EXPECT_EQ(RET_SUCCESS, res);
    EXPECT_EQ(RET_SUCCESS, AccessTokenKit::DeleteToken(fullTokenId.tokenIdExStruct.tokenID));
}

/**
 * @tc.name: UpdateHapToken009
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(AppInstallationOptimizedTest, UpdateHapToken009, TestSize.Level0)
{
    HapPolicyParams testPolicyParam = {
        .apl = APL_NORMAL,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerCameraState}
    };
    AccessTokenIDEx fullTokenId;
    int32_t res = AccessTokenKit::InitHapToken(g_testHapInfoParams, testPolicyParam, fullTokenId);
    EXPECT_EQ(RET_SUCCESS, res);

    HapPolicyParams testPolicyParam1 = {
        .apl = APL_NORMAL,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerCameraState, g_infoManagerTestState4},
    };
    UpdateHapInfoParams info = {
        .appIDDesc = "TEST",
        .apiVersion = DEFAULT_API_VERSION,
        .isSystemApp = true
    };
    res = AccessTokenKit::UpdateHapToken(fullTokenId, info, testPolicyParam1);
    EXPECT_NE(RET_SUCCESS, res);
    EXPECT_EQ(RET_SUCCESS, AccessTokenKit::DeleteToken(fullTokenId.tokenIdExStruct.tokenID));
}

/**
 * @tc.name: UpdateHapToken010
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(AppInstallationOptimizedTest, UpdateHapToken010, TestSize.Level0)
{
    HapPolicyParams testPolicyParams1 = {
        .apl = APL_NORMAL,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerCameraState}
    };
    AccessTokenIDEx fullTokenId;
    int32_t ret = AccessTokenKit::InitHapToken(g_testHapInfoParams, testPolicyParams1, fullTokenId);
    GTEST_LOG_(INFO) << "tokenID :" << fullTokenId.tokenIdExStruct.tokenID;
    EXPECT_EQ(RET_SUCCESS, ret);

    UpdateHapInfoParams info;
    info.appIDDesc = "TEST";
    info.apiVersion = DEFAULT_API_VERSION;
    info.isSystemApp = true;
    PreAuthorizationInfo info1 = {
        .permissionName = APP_TRACKING_PERMISSION,
        .userCancelable = false
    };
    HapPolicyParams testPolicyParams2 = {
        .apl = APL_NORMAL,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerCameraState},
        .preAuthorizationInfo = {info1}
    };
    ret = AccessTokenKit::UpdateHapToken(fullTokenId, info, testPolicyParams2);
    ASSERT_EQ(RET_SUCCESS, ret);
    std::vector<PermissionStateFull> state;
    int32_t res = TestCommon::GetReqPermissionsByTest(fullTokenId.tokenIdExStruct.tokenID, state, false);
    ASSERT_EQ(RET_SUCCESS, res);
    EXPECT_EQ(static_cast<uint32_t>(1), state.size());
    EXPECT_EQ(APP_TRACKING_PERMISSION, state[0].permissionName);
    EXPECT_EQ(state[0].grantStatus[0], PERMISSION_GRANTED);
    EXPECT_EQ(state[0].grantFlags[0], PERMISSION_SYSTEM_FIXED);

    EXPECT_EQ(RET_SUCCESS, AccessTokenKit::DeleteToken(fullTokenId.tokenIdExStruct.tokenID));
}

/**
 * @tc.name: UpdateHapToken011
 * @tc.desc: app user_grant permission has not been operated, update with pre-authorization
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(AppInstallationOptimizedTest, UpdateHapToken011, TestSize.Level0)
{
    HapPolicyParams testPolicyParams1 = {
        .apl = APL_NORMAL,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerCameraState}
    };
    AccessTokenIDEx fullTokenId;
    int32_t ret = AccessTokenKit::InitHapToken(g_testHapInfoParams, testPolicyParams1, fullTokenId);
    GTEST_LOG_(INFO) << "tokenID :" << fullTokenId.tokenIdExStruct.tokenID;
    EXPECT_EQ(RET_SUCCESS, ret);

    UpdateHapInfoParams info;
    info.appIDDesc = "TEST";
    info.apiVersion = DEFAULT_API_VERSION;
    info.isSystemApp = true;
    PreAuthorizationInfo info1 = {
        .permissionName = APP_TRACKING_PERMISSION,
        .userCancelable = false
    };
    HapPolicyParams testPolicyParams2 = {
        .apl = APL_NORMAL,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerCameraState},
        .preAuthorizationInfo = {info1}
    };
    ret = AccessTokenKit::UpdateHapToken(fullTokenId, info, testPolicyParams2);
    ASSERT_EQ(RET_SUCCESS, ret);
    std::vector<PermissionStateFull> state;
    int32_t res = TestCommon::GetReqPermissionsByTest(fullTokenId.tokenIdExStruct.tokenID, state, false);
    ASSERT_EQ(RET_SUCCESS, res);
    EXPECT_EQ(static_cast<uint32_t>(1), state.size());
    EXPECT_EQ(APP_TRACKING_PERMISSION, state[0].permissionName);
    EXPECT_EQ(state[0].grantStatus[0], PERMISSION_GRANTED);
    EXPECT_EQ(state[0].grantFlags[0], PERMISSION_SYSTEM_FIXED);

    EXPECT_EQ(RET_SUCCESS, AccessTokenKit::DeleteToken(fullTokenId.tokenIdExStruct.tokenID));
}

/**
 * @tc.name: UpdateHapToken012
 * @tc.desc: app user_grant permission has been granted or revoked by user, update with pre-authorization
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(AppInstallationOptimizedTest, UpdateHapToken012, TestSize.Level0)
{
    HapPolicyParams testPolicyParams1 = {
        .apl = APL_NORMAL,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerCameraState, g_infoManagerMicrophoneState}
    };
    AccessTokenIDEx fullTokenId;
    int32_t ret = AccessTokenKit::InitHapToken(g_testHapInfoParams, testPolicyParams1, fullTokenId);
    EXPECT_EQ(RET_SUCCESS, ret);

    ret = TestCommon::GrantPermissionByTest(
        fullTokenId.tokenIdExStruct.tokenID, APP_TRACKING_PERMISSION, PERMISSION_USER_FIXED);
    EXPECT_EQ(RET_SUCCESS, ret);
    ret = TestCommon::RevokePermissionByTest(
        fullTokenId.tokenIdExStruct.tokenID, CALENDAR_PERMISSION, PERMISSION_USER_FIXED);
    EXPECT_EQ(RET_SUCCESS, ret);

    UpdateHapInfoParams info;
    info.appIDDesc = "TEST";
    info.apiVersion = DEFAULT_API_VERSION;
    info.isSystemApp = true;
    PreAuthorizationInfo info1 = {
        .permissionName = APP_TRACKING_PERMISSION,
        .userCancelable = false
    };
    PreAuthorizationInfo info2 = {
        .permissionName = CALENDAR_PERMISSION,
        .userCancelable = false
    };
    HapPolicyParams testPolicyParams2 = {
        .apl = APL_NORMAL,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerCameraState, g_infoManagerMicrophoneState},
        .preAuthorizationInfo = {info1, info2}
    };
    ret = AccessTokenKit::UpdateHapToken(fullTokenId, info, testPolicyParams2);
    ASSERT_EQ(RET_SUCCESS, ret);
    std::vector<PermissionStateFull> state;
    TestCommon::GetReqPermissionsByTest(fullTokenId.tokenIdExStruct.tokenID, state, false);
    EXPECT_EQ(static_cast<uint32_t>(2), state.size());
    EXPECT_EQ(APP_TRACKING_PERMISSION, state[0].permissionName);
    EXPECT_EQ(state[0].grantStatus[0], PERMISSION_GRANTED);
    EXPECT_EQ(state[0].grantFlags[0], PERMISSION_USER_FIXED);
    EXPECT_EQ(CALENDAR_PERMISSION, state[1].permissionName);
    EXPECT_EQ(state[1].grantStatus[0], PERMISSION_DENIED);
    EXPECT_EQ(state[1].grantFlags[0], PERMISSION_USER_FIXED);

    EXPECT_EQ(RET_SUCCESS, AccessTokenKit::DeleteToken(fullTokenId.tokenIdExStruct.tokenID));
}


/**
 * @tc.name: UpdateHapToken013
 * @tc.desc: app user_grant permission has been pre-authorized with
 *           userUnCancelable flag, update with userCancelable pre-authorization
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(AppInstallationOptimizedTest, UpdateHapToken013, TestSize.Level0)
{
    PreAuthorizationInfo info1 = {
        .permissionName = APP_TRACKING_PERMISSION,
        .userCancelable = false
    };
    PreAuthorizationInfo info2 = {
        .permissionName = CALENDAR_PERMISSION,
        .userCancelable = false
    };
    HapPolicyParams testPolicyParams1 = {
        .apl = APL_NORMAL,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerCameraState, g_infoManagerMicrophoneState},
        .preAuthorizationInfo = {info1, info2}
    };
    AccessTokenIDEx fullTokenId;
    int32_t ret = AccessTokenKit::InitHapToken(g_testHapInfoParams, testPolicyParams1, fullTokenId);
    EXPECT_EQ(RET_SUCCESS, ret);

    ret = TestCommon::GrantPermissionByTest(
        fullTokenId.tokenIdExStruct.tokenID, APP_TRACKING_PERMISSION, PERMISSION_USER_FIXED);
    EXPECT_NE(RET_SUCCESS, ret);
    ret = TestCommon::RevokePermissionByTest(
        fullTokenId.tokenIdExStruct.tokenID, CALENDAR_PERMISSION, PERMISSION_USER_FIXED);
    EXPECT_NE(RET_SUCCESS, ret);

    UpdateHapInfoParams info;
    info.appIDDesc = "TEST";
    info.apiVersion = DEFAULT_API_VERSION;
    info.isSystemApp = true;
    info1.userCancelable = true;
    testPolicyParams1.preAuthorizationInfo = {info1};

    ret = AccessTokenKit::UpdateHapToken(fullTokenId, info, testPolicyParams1);
    ASSERT_EQ(RET_SUCCESS, ret);
    std::vector<PermissionStateFull> state;
    TestCommon::GetReqPermissionsByTest(fullTokenId.tokenIdExStruct.tokenID, state, false);
    EXPECT_EQ(static_cast<uint32_t>(2), state.size());
    EXPECT_EQ(state[0].grantStatus[0], PERMISSION_GRANTED);
    EXPECT_EQ(state[0].grantFlags[0], PERMISSION_PRE_AUTHORIZED_CANCELABLE);
    EXPECT_EQ(state[1].grantStatus[0], PERMISSION_DENIED);
    EXPECT_EQ(state[1].grantFlags[0], PERMISSION_DEFAULT_FLAG);

    EXPECT_EQ(RET_SUCCESS, AccessTokenKit::DeleteToken(fullTokenId.tokenIdExStruct.tokenID));
}

/**
 * @tc.name: UpdateHapToken014
 * @tc.desc: app user_grant permission has been pre-authorized with userCancelable flag,
 *           update with userCancelable pre-authorization
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(AppInstallationOptimizedTest, UpdateHapToken014, TestSize.Level0)
{
    PreAuthorizationInfo info1 = {
        .permissionName = APP_TRACKING_PERMISSION,
        .userCancelable = true
    };
    PreAuthorizationInfo info2 = {
        .permissionName = CALENDAR_PERMISSION,
        .userCancelable = true
    };
    HapPolicyParams testPolicyParams1 = {
        .apl = APL_NORMAL,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerCameraState, g_infoManagerMicrophoneState},
        .preAuthorizationInfo = {info1, info2}
    };
    AccessTokenIDEx fullTokenId;
    int32_t ret = AccessTokenKit::InitHapToken(g_testHapInfoParams, testPolicyParams1, fullTokenId);
    EXPECT_EQ(RET_SUCCESS, ret);

    ret = TestCommon::RevokePermissionByTest(
        fullTokenId.tokenIdExStruct.tokenID, CALENDAR_PERMISSION, PERMISSION_USER_FIXED);
    EXPECT_EQ(RET_SUCCESS, ret);

    UpdateHapInfoParams info;
    info.appIDDesc = "TEST";
    info.apiVersion = DEFAULT_API_VERSION;
    info.isSystemApp = true;
    info1.userCancelable = false;
    testPolicyParams1.preAuthorizationInfo = {info1};

    ret = AccessTokenKit::UpdateHapToken(fullTokenId, info, testPolicyParams1);
    ASSERT_EQ(RET_SUCCESS, ret);
    std::vector<PermissionStateFull> state;
    TestCommon::GetReqPermissionsByTest(fullTokenId.tokenIdExStruct.tokenID, state, false);
    EXPECT_EQ(static_cast<uint32_t>(2), state.size());
    EXPECT_EQ(state[0].grantStatus[0], PERMISSION_GRANTED);
    EXPECT_EQ(state[0].grantFlags[0], PERMISSION_SYSTEM_FIXED);
    EXPECT_EQ(state[1].grantStatus[0], PERMISSION_DENIED);
    EXPECT_EQ(state[1].grantFlags[0], PERMISSION_USER_FIXED | PERMISSION_PRE_AUTHORIZED_CANCELABLE);

    EXPECT_EQ(RET_SUCCESS, AccessTokenKit::DeleteToken(fullTokenId.tokenIdExStruct.tokenID));
}

/**
 * @tc.name: InitHapTokenAbnormal001
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(AppInstallationOptimizedTest, InitHapTokenAbnormal001, TestSize.Level0)
{
    HapPolicyParams testPolicyParam = {
        .apl = APL_NORMAL,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerCameraState},
        .aclRequestedList = {}
    };
    for (uint32_t i = 0; i < MAX_PERM_LIST_SIZE; i++) {
        testPolicyParam.aclRequestedList.emplace_back("ohos.permission.CAMERA");
    }
    AccessTokenIDEx fullTokenId;
    int32_t res = AccessTokenKit::InitHapToken(g_testHapInfoParams, testPolicyParam, fullTokenId);
    EXPECT_EQ(RET_SUCCESS, res);
    ASSERT_EQ(RET_SUCCESS, AccessTokenKit::DeleteToken(fullTokenId.tokenIdExStruct.tokenID));
    testPolicyParam.aclRequestedList.emplace_back("ohos.permission.CAMERA");

    res = AccessTokenKit::InitHapToken(g_testHapInfoParams, testPolicyParam, fullTokenId);
    EXPECT_NE(RET_SUCCESS, res);
}

/**
 * @tc.name: InitHapTokenAbnormal002
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(AppInstallationOptimizedTest, InitHapTokenAbnormal002, TestSize.Level0)
{
    HapPolicyParams testPolicyParam = {
        .apl = APL_NORMAL,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerCameraState},
        .aclRequestedList = {},
    };
    PreAuthorizationInfo info = {
        .permissionName = CALENDAR_PERMISSION,
        .userCancelable = false
    };
    for (uint32_t i = 0; i < MAX_PERM_LIST_SIZE; i++) {
        testPolicyParam.preAuthorizationInfo.emplace_back(info);
    }
    AccessTokenIDEx fullTokenId;
    int32_t res = AccessTokenKit::InitHapToken(g_testHapInfoParams, testPolicyParam, fullTokenId);
    EXPECT_EQ(RET_SUCCESS, res);
    ASSERT_EQ(RET_SUCCESS, AccessTokenKit::DeleteToken(fullTokenId.tokenIdExStruct.tokenID));
    testPolicyParam.preAuthorizationInfo.emplace_back(info);

    res = AccessTokenKit::InitHapToken(g_testHapInfoParams, testPolicyParam, fullTokenId);
    EXPECT_NE(RET_SUCCESS, res);
}

/**
 * @tc.name: InitHapTokenAbnormal003
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(AppInstallationOptimizedTest, InitHapTokenAbnormal003, TestSize.Level0)
{
    HapInfoParams testHapInfoParams = g_testHapInfoParams;
    HapPolicyParams testPolicyParam = {
        .apl = APL_NORMAL,
        .domain = "test.domain2",
        .permStateList = {g_infoManagerCameraState},
        .aclRequestedList = {},
        .preAuthorizationInfo = {}
    };

    // invalid userID
    testHapInfoParams.userID = -1;
    AccessTokenIDEx fullTokenId;
    int32_t res = AccessTokenKit::InitHapToken(testHapInfoParams, testPolicyParam, fullTokenId);
    EXPECT_EQ(ERR_PARAM_INVALID, res);
    testHapInfoParams.userID = g_testHapInfoParams.userID;

    // invalid bundleName
    testHapInfoParams.bundleName = "";
    res = AccessTokenKit::InitHapToken(testHapInfoParams, testPolicyParam, fullTokenId);
    EXPECT_EQ(ERR_PARAM_INVALID, res);
    testHapInfoParams.bundleName = g_testHapInfoParams.bundleName;

    // invalid dlpType
    testHapInfoParams.dlpType = -1;
    res = AccessTokenKit::InitHapToken(testHapInfoParams, testPolicyParam, fullTokenId);
    EXPECT_EQ(ERR_PARAM_INVALID, res);
    testHapInfoParams.dlpType = g_testHapInfoParams.dlpType;

    int32_t invalidAppIdLen = 10241; // 10241 is invalid appid length
    // invalid dlpType
    std::string invalidAppIDDesc (invalidAppIdLen, 'x');
    testHapInfoParams.appIDDesc = invalidAppIDDesc;
    res = AccessTokenKit::InitHapToken(testHapInfoParams, testPolicyParam, fullTokenId);
    EXPECT_EQ(ERR_PARAM_INVALID, res);
    testHapInfoParams.appIDDesc = g_testHapInfoParams.appIDDesc;
}

/**
 * @tc.name: InitHapTokenAbnormal004
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(AppInstallationOptimizedTest, InitHapTokenAbnormal004, TestSize.Level0)
{
    HapInfoParams testHapInfoParams = g_testHapInfoParams;
    HapPolicyParams testPolicyParam = g_testPolicyParams;

    // invalid apl 8
    testPolicyParam.apl = static_cast<AccessToken::TypeATokenAplEnum>(8);
    AccessTokenIDEx fullTokenId;
    int32_t res = AccessTokenKit::InitHapToken(testHapInfoParams, testPolicyParam, fullTokenId);
    EXPECT_NE(RET_SUCCESS, res);
    testPolicyParam.apl = g_testPolicyParams.apl;

    // invalid domain 1025
    const static int32_t MAX_DCAP_LENGTH = 1025;
    std::string invalidDomain (MAX_DCAP_LENGTH, 'x');
    testPolicyParam.domain = invalidDomain;
    res = AccessTokenKit::InitHapToken(testHapInfoParams, testPolicyParam, fullTokenId);
    EXPECT_NE(RET_SUCCESS, res);
    testPolicyParam.domain = g_testPolicyParams.domain;
}

} // namespace AccessToken
} // namespace Security
} // namespace OHOS
