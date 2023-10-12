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

#include "privacy_kit_test.h"

#include <chrono>

#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "on_permission_used_record_callback_stub.h"
#include "parameter.h"
#define private public
#include "perm_active_status_change_callback.h"
#include "privacy_manager_client.h"
#include "state_change_callback.h"
#undef private
#include "perm_active_response_parcel.h"
#include "perm_active_status_change_callback_stub.h"
#include "privacy_error.h"
#include "privacy_kit.h"
#include "state_change_callback_stub.h"
#include "string_ex.h"
#include "token_setproc.h"

using namespace testing::ext;
using namespace OHOS::Security::AccessToken;

const static int32_t RET_NO_ERROR = 0;
static constexpr int32_t DEFAULT_API_VERSION = 8;

static PermissionStateFull g_infoManagerTestStateA = {
    .permissionName = "ohos.permission.CAMERA",
    .isGeneral = true,
    .resDeviceID = {"local"},
    .grantStatus = {PermissionState::PERMISSION_GRANTED},
    .grantFlags = {1}
};
static HapPolicyParams g_policyPramsA = {
    .apl = APL_NORMAL,
    .domain = "test.domain.A",
};
static HapInfoParams g_infoParmsA = {
    .userID = 1,
    .bundleName = "ohos.privacy_test.bundleA",
    .instIndex = 0,
    .appIDDesc = "privacy_test.bundleA"
};

static PermissionStateFull g_infoManagerTestStateB = {
    .permissionName = "ohos.permission.MICROPHONE",
    .isGeneral = true,
    .resDeviceID = {"local"},
    .grantStatus = {PermissionState::PERMISSION_GRANTED},
    .grantFlags = {1}
};
static HapPolicyParams g_policyPramsB = {
    .apl = APL_NORMAL,
    .domain = "test.domain.B",
};
static HapInfoParams g_infoParmsB = {
    .userID = 1,
    .bundleName = "ohos.privacy_test.bundleB",
    .instIndex = 0,
    .appIDDesc = "privacy_test.bundleB"
};

static PermissionDef g_infoManagerTestPermDefC = {
    .permissionName = "ohos.permission.PERMISSION_USED_STATS",
    .bundleName = "ohos.privacy_test.bundleC",
    .grantMode = 1,
    .availableLevel = APL_NORMAL,
    .label = "labelC",
    .labelId = 1,
    .description = "break the door",
    .descriptionId = 1
};
static PermissionStateFull g_infoManagerTestStateC = {
    .permissionName = "ohos.permission.PERMISSION_USED_STATS",
    .isGeneral = true,
    .resDeviceID = {"localC"},
    .grantStatus = {PermissionState::PERMISSION_GRANTED},
    .grantFlags = {1}
};
static HapInfoParams g_infoParmsC = {
    .userID = 1,
    .bundleName = "ohos.privacy_test.bundleC",
    .instIndex = 0,
    .appIDDesc = "privacy_test.bundleC",
    .isSystemApp = true,
};
static HapPolicyParams g_policyPramsC = {
    .apl = APL_NORMAL,
    .domain = "test.domain.C",
    .permList = {g_infoManagerTestPermDefC},
    .permStateList = {g_infoManagerTestStateC}
};

static HapPolicyParams g_policyPramsE = {
    .apl = APL_NORMAL,
    .domain = "test.domain",
    .permList = {},
    .permStateList = {g_infoManagerTestStateA, g_infoManagerTestStateB}
};
static HapInfoParams g_infoParmsE = {
    .userID = 1,
    .bundleName = "ohos.privacy_test.bundleE",
    .instIndex = 0,
    .appIDDesc = "privacy_test.bundleE"
};

static UsedRecordDetail g_usedRecordDetail = {
    .status = 2,
    .timestamp = 2L,
    .accessDuration = 2L,
};
static PermissionUsedRecord g_permissionUsedRecord = {
    .permissionName = "ohos.permission.CAMERA",
    .accessCount = 2,
    .rejectCount = 2,
    .lastAccessTime = 0L,
    .lastRejectTime = 0L,
    .lastAccessDuration = 0L,
};
static BundleUsedRecord g_bundleUsedRecord = {
    .tokenId = 100,
    .isRemote = false,
    .deviceId = "you guess",
    .bundleName = "com.ohos.test",
};

static HapInfoParams g_normalInfoParms = {
    .userID = 1,
    .bundleName = "accesstoken_test",
    .instIndex = 0,
    .appIDDesc = "testtesttesttest",
    .apiVersion = DEFAULT_API_VERSION,
    .isSystemApp = false
};
static HapInfoParams g_systemInfoParms = {
    .userID = 1,
    .bundleName = "accesstoken_test",
    .instIndex = 0,
    .appIDDesc = "testtesttesttest",
    .apiVersion = DEFAULT_API_VERSION,
    .isSystemApp = true
};

static AccessTokenID g_selfTokenId = 0;
static AccessTokenID g_tokenIdA = 0;
static AccessTokenID g_tokenIdB = 0;
static AccessTokenIDEx g_tokenIdC = {0};
static AccessTokenID g_tokenIdE = 0;

static void DeleteTestToken()
{
    AccessTokenID tokenId = AccessTokenKit::GetHapTokenID(g_infoParmsA.userID,
                                                          g_infoParmsA.bundleName,
                                                          g_infoParmsA.instIndex);
    AccessTokenKit::DeleteToken(tokenId);

    tokenId = AccessTokenKit::GetHapTokenID(g_infoParmsB.userID,
                                            g_infoParmsB.bundleName,
                                            g_infoParmsB.instIndex);
    AccessTokenKit::DeleteToken(tokenId);

    tokenId = AccessTokenKit::GetHapTokenID(g_infoParmsC.userID,
                                            g_infoParmsC.bundleName,
                                            g_infoParmsC.instIndex);
    AccessTokenKit::DeleteToken(tokenId);

    tokenId = AccessTokenKit::GetHapTokenID(g_infoParmsE.userID,
                                            g_infoParmsE.bundleName,
                                            g_infoParmsE.instIndex);
    AccessTokenKit::DeleteToken(tokenId);

    tokenId = AccessTokenKit::GetHapTokenID(g_normalInfoParms.userID,
                                            g_normalInfoParms.bundleName,
                                            g_normalInfoParms.instIndex);
    AccessTokenKit::DeleteToken(tokenId);

    tokenId = AccessTokenKit::GetHapTokenID(g_systemInfoParms.userID,
                                            g_systemInfoParms.bundleName,
                                            g_systemInfoParms.instIndex);
    AccessTokenKit::DeleteToken(tokenId);
}

void PrivacyKitTest::SetUpTestCase()
{
    DeleteTestToken();
    g_selfTokenId = GetSelfTokenID();
}

void PrivacyKitTest::TearDownTestCase()
{
}

void PrivacyKitTest::SetUp()
{
    AccessTokenKit::AllocHapToken(g_infoParmsA, g_policyPramsA);
    AccessTokenKit::AllocHapToken(g_infoParmsB, g_policyPramsB);
    AccessTokenKit::AllocHapToken(g_infoParmsC, g_policyPramsC);
    AccessTokenKit::AllocHapToken(g_infoParmsE, g_policyPramsE);

    g_tokenIdA = AccessTokenKit::GetHapTokenID(g_infoParmsA.userID, g_infoParmsA.bundleName, g_infoParmsA.instIndex);
    g_tokenIdB = AccessTokenKit::GetHapTokenID(g_infoParmsB.userID, g_infoParmsB.bundleName, g_infoParmsB.instIndex);
    g_tokenIdC = AccessTokenKit::GetHapTokenIDEx(g_infoParmsC.userID, g_infoParmsC.bundleName, g_infoParmsC.instIndex);
    g_tokenIdE = AccessTokenKit::GetHapTokenID(g_infoParmsE.userID, g_infoParmsE.bundleName, g_infoParmsE.instIndex);
}

void PrivacyKitTest::TearDown()
{
    EXPECT_EQ(0, SetSelfTokenID(g_selfTokenId));
    DeleteTestToken();
}

std::string PrivacyKitTest::GetLocalDeviceUdid()
{
    const int32_t DEVICE_UUID_LENGTH = 65;
    char udid[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(udid, DEVICE_UUID_LENGTH);
    return udid;
}

void PrivacyKitTest::BuildQueryRequest(AccessTokenID tokenId, const std::string &deviceId,
    const std::string &bundleName, const std::vector<std::string> &permissionList, PermissionUsedRequest &request)
{
    request.tokenId = tokenId;
    request.isRemote = false;
    request.deviceId = deviceId;
    request.bundleName = bundleName;
    request.permissionList = permissionList;
    request.beginTimeMillis = 0;
    request.endTimeMillis = 0;
    request.flag = FLAG_PERMISSION_USAGE_SUMMARY;
}

void PrivacyKitTest::CheckPermissionUsedResult(const PermissionUsedRequest& request, const PermissionUsedResult& result,
    int32_t permRecordSize, int32_t totalSuccessCount, int32_t totalFailCount)
{
    int32_t successCount = 0;
    int32_t failCount = 0;
    ASSERT_EQ(request.tokenId, result.bundleRecords[0].tokenId);
    ASSERT_EQ(request.isRemote, result.bundleRecords[0].isRemote);
    ASSERT_EQ(request.deviceId, result.bundleRecords[0].deviceId);
    ASSERT_EQ(request.bundleName, result.bundleRecords[0].bundleName);
    ASSERT_EQ(permRecordSize, static_cast<int32_t>(result.bundleRecords[0].permissionRecords.size()));
    for (int32_t i = 0; i < permRecordSize; i++) {
        successCount += result.bundleRecords[0].permissionRecords[i].accessCount;
        failCount += result.bundleRecords[0].permissionRecords[i].rejectCount;
    }
    ASSERT_EQ(totalSuccessCount, successCount);
    ASSERT_EQ(totalFailCount, failCount);
}

static void SetTokenID(std::vector<HapInfoParams>& g_InfoParms_List,
    std::vector<AccessTokenID>& g_TokenId_List, int32_t number)
{
    EXPECT_EQ(0, SetSelfTokenID(g_selfTokenId));
    for (int32_t i = 0; i < number; i++) {
        HapInfoParams g_InfoParmsTmp = {
            .userID = i,
            .bundleName = "ohos.privacy_test.bundle" + std::to_string(i),
            .instIndex = i,
            .appIDDesc = "privacy_test.bundle" + std::to_string(i)
        };
        g_InfoParms_List.push_back(g_InfoParmsTmp);
        HapPolicyParams g_PolicyPramsTmp = {
            .apl = APL_NORMAL,
            .domain = "test.domain." + std::to_string(i)
        };
        AccessTokenKit::AllocHapToken(g_InfoParmsTmp, g_PolicyPramsTmp);
        AccessTokenID g_TokenId_Tmp = AccessTokenKit::GetHapTokenID(g_InfoParmsTmp.userID,
                                                                    g_InfoParmsTmp.bundleName,
                                                                    g_InfoParmsTmp.instIndex);
        g_TokenId_List.push_back(g_TokenId_Tmp);
    }

    EXPECT_EQ(0, SetSelfTokenID(g_tokenIdC.tokenIDEx));
}

static void DeleteTokenID(std::vector<HapInfoParams>& g_InfoParms_List)
{
    EXPECT_EQ(0, SetSelfTokenID(g_selfTokenId));
    for (size_t i = 0; i < g_InfoParms_List.size(); i++) {
        AccessTokenID g_TokenId_Tmp = AccessTokenKit::GetHapTokenID(g_InfoParms_List[i].userID,
                                                                    g_InfoParms_List[i].bundleName,
                                                                    g_InfoParms_List[i].instIndex);
        AccessTokenKit::DeleteToken(g_TokenId_Tmp);
    }

    EXPECT_EQ(0, SetSelfTokenID(g_tokenIdC.tokenIDEx));
}

void SleepUtilMinuteEnd()
{
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    );

    int64_t timestamp_ms = ms.count();
    time_t timestamp_s = static_cast<time_t>(timestamp_ms / 1000);
    struct tm t = {0};
    // localtime is not thread safe, localtime_r first param unit is second, timestamp unit is ms, so divided by 1000
    localtime_r(&timestamp_s, &t);
    uint32_t sleepSeconds = static_cast<uint32_t>(60 - t.tm_sec);

    GTEST_LOG_(INFO) << "current time is " << timestamp_ms << ", " << t.tm_hour << ":" << t.tm_min << ":" << t.tm_sec;
    GTEST_LOG_(INFO) << "need to sleep " << sleepSeconds << " seconds";
    sleep(sleepSeconds);
}

/**
 * @tc.name: AddPermissionUsedRecord001
 * @tc.desc: cannot AddPermissionUsedRecord with illegal tokenId and permission.
 * @tc.type: FUNC
 * @tc.require: issueI5P4IU
 */
HWTEST_F(PrivacyKitTest, AddPermissionUsedRecord001, TestSize.Level1)
{
    ASSERT_EQ(PrivacyError::ERR_PARAM_INVALID, PrivacyKit::AddPermissionUsedRecord(
        0, "ohos.permission.READ_CONTACTS", 1, 0));
    ASSERT_EQ(PrivacyError::ERR_PARAM_INVALID, PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, "", 1, 0));
    ASSERT_EQ(PrivacyError::ERR_PARAM_INVALID, PrivacyKit::AddPermissionUsedRecord(
        g_tokenIdA, "ohos.permission.READ_CONTACTS", -1, 0));

    PermissionUsedRequest request;
    PermissionUsedResult result;
    std::vector<std::string> permissionList;
    BuildQueryRequest(g_tokenIdA, GetLocalDeviceUdid(), g_infoParmsA.bundleName, permissionList, request);
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::GetPermissionUsedRecords(request, result));
    ASSERT_EQ(static_cast<uint32_t>(0), result.bundleRecords.size());
}

/**
 * @tc.name: AddPermissionUsedRecord002
 * @tc.desc: cannot AddPermissionUsedRecord with invalid tokenId and permission.
 * @tc.type: FUNC
 * @tc.require: issueI5P4IU
 */
HWTEST_F(PrivacyKitTest, AddPermissionUsedRecord002, TestSize.Level1)
{
    ASSERT_EQ(PrivacyError::ERR_PERMISSION_NOT_EXIST,
        PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, "ohos.permission.test", 1, 0));
    ASSERT_EQ(PrivacyError::ERR_TOKENID_NOT_EXIST,
        PrivacyKit::AddPermissionUsedRecord(123, "ohos.permission.CAMERA", 1, 0));
    ASSERT_EQ(PrivacyError::ERR_PARAM_INVALID,
        PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, "ohos.permission.READ_CONTACTS", 0, 0));

    PermissionUsedRequest request;
    PermissionUsedResult result;
    std::vector<std::string> permissionList;
    BuildQueryRequest(123, "", "", permissionList, request);
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::GetPermissionUsedRecords(request, result));
    ASSERT_EQ(static_cast<uint32_t>(0), result.bundleRecords.size());

    BuildQueryRequest(g_tokenIdA, GetLocalDeviceUdid(), g_infoParmsA.bundleName, permissionList, request);
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::GetPermissionUsedRecords(request, result));
    ASSERT_EQ(static_cast<uint32_t>(0), result.bundleRecords.size());
}

/**
 * @tc.name: AddPermissionUsedRecord003
 * @tc.desc: cannot AddPermissionUsedRecord with native tokenId.
 * @tc.type: FUNC
 * @tc.require: issueI5P4IU
 */
HWTEST_F(PrivacyKitTest, AddPermissionUsedRecord003, TestSize.Level1)
{
    const char **dcaps = new const char *[2];
    dcaps[0] = "AT_CAP";
    dcaps[1] = "ST_CAP";
    uint64_t tokenId;
    const char **acls = new const char *[2];
    acls[0] = "ohos.permission.test1";
    acls[1] = "ohos.permission.test2";
    const char **perms = new const char *[2];
    perms[0] = "ohos.permission.test1";
    perms[1] = "ohos.permission.test2";
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 2,
        .permsNum = 2,
        .aclsNum = 2,
        .dcaps = dcaps,
        .perms = perms,
        .acls = acls,
        .processName = "GetAccessTokenId008",
        .aplStr = "system_core",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    ASSERT_NE(tokenId, static_cast<uint32_t>(0));

    delete[] perms;
    delete[] dcaps;
    delete[] acls;

    ASSERT_EQ(PrivacyError::ERR_TOKENID_NOT_EXIST, PrivacyKit::AddPermissionUsedRecord(
        tokenId, "ohos.permission.READ_CONTACTS", 1, 0));

    PermissionUsedRequest request;
    PermissionUsedResult result;
    std::vector<std::string> permissionList;
    BuildQueryRequest(tokenId, "", "", permissionList, request);

    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::GetPermissionUsedRecords(request, result));
    ASSERT_EQ(static_cast<uint32_t>(0), result.bundleRecords.size());
}

/**
 * @tc.name: AddPermissionUsedRecord004
 * @tc.desc: AddPermissionUsedRecord user_grant permission.
 * @tc.type: FUNC
 * @tc.require: issueI5P4IU
 */
HWTEST_F(PrivacyKitTest, AddPermissionUsedRecord004, TestSize.Level1)
{
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, "ohos.permission.CAMERA", 1, 0));
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, "ohos.permission.WRITE_CONTACTS", 0, 1));
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, "ohos.permission.LOCATION", 1, 1));

    PermissionUsedRequest request;
    PermissionUsedResult result;
    std::vector<std::string> permissionList;
    BuildQueryRequest(g_tokenIdA, GetLocalDeviceUdid(), g_infoParmsA.bundleName, permissionList, request);
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::GetPermissionUsedRecords(request, result));

    ASSERT_EQ(static_cast<uint32_t>(1), result.bundleRecords.size());
    CheckPermissionUsedResult(request, result, 3, 2, 2);
}

/**
 * @tc.name: AddPermissionUsedRecord005
 * @tc.desc: AddPermissionUsedRecord user_grant permission.
 * @tc.type: FUNC
 * @tc.require: issueI5P4IU
 */
HWTEST_F(PrivacyKitTest, AddPermissionUsedRecord005, TestSize.Level1)
{
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, "ohos.permission.CAMERA", 1, 0));
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, "ohos.permission.LOCATION", 0, 1));

    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdB,  "ohos.permission.CAMERA", 0, 1));
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdB,  "ohos.permission.LOCATION", 1, 0));


    PermissionUsedRequest request;
    PermissionUsedResult result;
    std::vector<std::string> permissionList;
    BuildQueryRequest(g_tokenIdA, GetLocalDeviceUdid(), g_infoParmsA.bundleName, permissionList, request);
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::GetPermissionUsedRecords(request, result));

    ASSERT_EQ(static_cast<uint32_t>(1), result.bundleRecords.size());
    CheckPermissionUsedResult(request, result, 2, 1, 1);

    BuildQueryRequest(g_tokenIdB, GetLocalDeviceUdid(), g_infoParmsB.bundleName, permissionList, request);
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::GetPermissionUsedRecords(request, result));

    ASSERT_EQ(static_cast<uint32_t>(1), result.bundleRecords.size());
    CheckPermissionUsedResult(request, result, 2, 1, 1);
}

/**
 * @tc.name: AddPermissionUsedRecord006
 * @tc.desc: AddPermissionUsedRecord permission combine records.
 * @tc.type: FUNC
 * @tc.require: issueI5P4IU
 */
HWTEST_F(PrivacyKitTest, AddPermissionUsedRecord006, TestSize.Level1)
{
    SleepUtilMinuteEnd();
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, "ohos.permission.CAMERA", 1, 0));
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, "ohos.permission.CAMERA", 1, 0));
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, "ohos.permission.CAMERA", 1, 0));
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, "ohos.permission.CAMERA", 1, 0));

    PermissionUsedRequest request;
    PermissionUsedResult result;
    std::vector<std::string> permissionList;
    BuildQueryRequest(g_tokenIdA, GetLocalDeviceUdid(), g_infoParmsA.bundleName, permissionList, request);
    request.flag = FLAG_PERMISSION_USAGE_DETAIL;
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::GetPermissionUsedRecords(request, result));

    ASSERT_EQ(static_cast<uint32_t>(1), result.bundleRecords.size());
    ASSERT_EQ(static_cast<uint32_t>(1), result.bundleRecords[0].permissionRecords.size());
    ASSERT_EQ(static_cast<uint32_t>(1), result.bundleRecords[0].permissionRecords[0].accessRecords.size());
    CheckPermissionUsedResult(request, result, 1, 4, 0);

    sleep(61);

    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, "ohos.permission.CAMERA", 1, 0));
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::GetPermissionUsedRecords(request, result));
    ASSERT_EQ(static_cast<uint32_t>(1), result.bundleRecords.size());
    ASSERT_EQ(static_cast<uint32_t>(1), result.bundleRecords[0].permissionRecords.size());
    ASSERT_EQ(static_cast<uint32_t>(2), result.bundleRecords[0].permissionRecords[0].accessRecords.size());
    CheckPermissionUsedResult(request, result, 1, 5, 0);
}

/**
 * @tc.name: AddPermissionUsedRecord007
 * @tc.desc: AddPermissionUsedRecord user_grant permission.
 * @tc.type: FUNC
 * @tc.require: issueI5P4IU
 */
HWTEST_F(PrivacyKitTest, AddPermissionUsedRecord007, TestSize.Level1)
{
    std::vector<HapInfoParams> g_InfoParms_List;
    std::vector<AccessTokenID> g_TokenId_List;
    SetTokenID(g_InfoParms_List, g_TokenId_List, 100);
    std::vector<std::string> addPermissionList = {
        "ohos.permission.ANSWER_CALL",
        "ohos.permission.READ_CALENDAR",
    };
    for (int32_t i = 0; i < 200; i++) {
        ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_TokenId_List[i % 100],
            addPermissionList[i % 2], 1, 0));

        PermissionUsedRequest request;
        PermissionUsedResult result;
        std::vector<std::string> permissionList;
        BuildQueryRequest(g_TokenId_List[i % 100], GetLocalDeviceUdid(),
            g_InfoParms_List[i % 100].bundleName, permissionList, request);
        request.flag = FLAG_PERMISSION_USAGE_DETAIL;
        ASSERT_EQ(RET_NO_ERROR, PrivacyKit::GetPermissionUsedRecords(request, result));
    }
    sleep(70);
    for (int32_t i = 0; i < 100; i++) {
        PermissionUsedRequest request;
        PermissionUsedResult result;
        std::vector<std::string> permissionList;
        BuildQueryRequest(g_TokenId_List[i], GetLocalDeviceUdid(),
            g_InfoParms_List[i].bundleName, permissionList, request);
        request.flag = FLAG_PERMISSION_USAGE_DETAIL;

        ASSERT_EQ(RET_NO_ERROR, PrivacyKit::GetPermissionUsedRecords(request, result));
        ASSERT_EQ(static_cast<uint32_t>(1), result.bundleRecords.size());
        ASSERT_EQ(static_cast<uint32_t>(1), result.bundleRecords[0].permissionRecords.size());
        ASSERT_EQ(static_cast<uint32_t>(2), result.bundleRecords[0].permissionRecords[0].accessRecords.size());
        CheckPermissionUsedResult(request, result, 1, 2, 0);
    }
    DeleteTokenID(g_InfoParms_List);
}

/**
 * @tc.name: AddPermissionUsedRecord008
 * @tc.desc: AddPermissionUsedRecord caller is normal app.
 * @tc.type: FUNC
 * @tc.require: issueI66BH3
 */
HWTEST_F(PrivacyKitTest, AddPermissionUsedRecord008, TestSize.Level1)
{
    AccessTokenIDEx tokenIdEx = {0};
    tokenIdEx = AccessTokenKit::AllocHapToken(g_normalInfoParms, g_policyPramsA);
    ASSERT_NE(INVALID_TOKENID, tokenIdEx.tokenIDEx);
    EXPECT_EQ(0, SetSelfTokenID(tokenIdEx.tokenIDEx));

    ASSERT_EQ(PrivacyError::ERR_NOT_SYSTEM_APP, PrivacyKit::AddPermissionUsedRecord(g_tokenIdA,
        "ohos.permission.CAMERA", 1, 0));
    ASSERT_EQ(0, PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, "ohos.permission.CAMERA", 1, 0, true));
}

/**
 * @tc.name: AddPermissionUsedRecord009
 * @tc.desc: query permission record detail count.
 * @tc.type: FUNC
 * @tc.require: issueI5P4IU
 */
HWTEST_F(PrivacyKitTest, AddPermissionUsedRecord009, TestSize.Level1)
{
    int32_t permRecordSize = 0;
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, "ohos.permission.CAMERA", 1, 0));
    permRecordSize++;
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, "ohos.permission.WRITE_CONTACTS", 0, 2));
    permRecordSize++;
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, "ohos.permission.LOCATION", 3, 3));
    permRecordSize++;

    PermissionUsedRequest request;
    PermissionUsedResult result;
    std::vector<std::string> permissionList;
    BuildQueryRequest(g_tokenIdA, GetLocalDeviceUdid(), g_infoParmsA.bundleName, permissionList, request);
    request.flag = FLAG_PERMISSION_USAGE_DETAIL;
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::GetPermissionUsedRecords(request, result));

    ASSERT_EQ(static_cast<uint32_t>(1), result.bundleRecords.size());
    ASSERT_EQ(permRecordSize, static_cast<int32_t>(result.bundleRecords[0].permissionRecords.size()));
    for (int32_t i = 0; i < permRecordSize; i++) {
        if (result.bundleRecords[0].permissionRecords[i].permissionName == "ohos.permission.CAMERA") {
            ASSERT_EQ(static_cast<uint32_t>(1), result.bundleRecords[0].permissionRecords[i].accessRecords.size());
            ASSERT_EQ(static_cast<uint32_t>(0), result.bundleRecords[0].permissionRecords[i].rejectRecords.size());
            ASSERT_EQ(1, result.bundleRecords[0].permissionRecords[i].accessRecords[0].count);
        } else if (result.bundleRecords[0].permissionRecords[i].permissionName == "ohos.permission.WRITE_CONTACTS") {
            ASSERT_EQ(static_cast<uint32_t>(0), result.bundleRecords[0].permissionRecords[i].accessRecords.size());
            ASSERT_EQ(static_cast<uint32_t>(1), result.bundleRecords[0].permissionRecords[i].rejectRecords.size());
            ASSERT_EQ(2, result.bundleRecords[0].permissionRecords[i].rejectRecords[0].count);
        } else if (result.bundleRecords[0].permissionRecords[i].permissionName == "ohos.permission.LOCATION") {
            ASSERT_EQ(static_cast<uint32_t>(1), result.bundleRecords[0].permissionRecords[i].accessRecords.size());
            ASSERT_EQ(static_cast<uint32_t>(1), result.bundleRecords[0].permissionRecords[i].rejectRecords.size());
            ASSERT_EQ(3, result.bundleRecords[0].permissionRecords[i].accessRecords[0].count);
            ASSERT_EQ(3, result.bundleRecords[0].permissionRecords[i].rejectRecords[0].count);
        }
    }
}

/**
 * @tc.name: AddPermissionUsedRecord010
 * @tc.desc: test record cross minute not merge.
 * @tc.type: FUNC
 * @tc.require: issueI5P4IU
 */
HWTEST_F(PrivacyKitTest, AddPermissionUsedRecord010, TestSize.Level1)
{
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, "ohos.permission.CAMERA", 1, 0));
    SleepUtilMinuteEnd();
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, "ohos.permission.CAMERA", 1, 0));

    PermissionUsedRequest request;
    PermissionUsedResult result;
    std::vector<std::string> permissionList;
    BuildQueryRequest(g_tokenIdA, GetLocalDeviceUdid(), g_infoParmsA.bundleName, permissionList, request);
    request.flag = FLAG_PERMISSION_USAGE_DETAIL;
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::GetPermissionUsedRecords(request, result));

    ASSERT_EQ(static_cast<uint32_t>(1), result.bundleRecords.size());
    ASSERT_EQ(static_cast<uint32_t>(1), result.bundleRecords[0].permissionRecords.size());
    ASSERT_EQ(static_cast<uint32_t>(2), result.bundleRecords[0].permissionRecords[0].accessRecords.size());
}

/**
 * @tc.name: RemovePermissionUsedRecords001
 * @tc.desc: cannot RemovePermissionUsedRecords with illegal tokenId and deviceID.
 * @tc.type: FUNC
 * @tc.require: issueI5P4IU
 */
HWTEST_F(PrivacyKitTest, RemovePermissionUsedRecords001, TestSize.Level1)
{
    ASSERT_EQ(PrivacyError::ERR_PARAM_INVALID, PrivacyKit::RemovePermissionUsedRecords(0, ""));
}

/**
 * @tc.name: RemovePermissionUsedRecords002
 * @tc.desc: RemovePermissionUsedRecords with invalid tokenId and deviceID.
 * @tc.type: FUNC
 * @tc.require: issueI5P4IU
 */
HWTEST_F(PrivacyKitTest, RemovePermissionUsedRecords002, TestSize.Level1)
{
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, "ohos.permission.CAMERA", 1, 0));
    PermissionUsedRequest request;
    PermissionUsedResult result;
    std::vector<std::string> permissionList;
    BuildQueryRequest(g_tokenIdA, GetLocalDeviceUdid(), g_infoParmsA.bundleName, permissionList, request);

    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::RemovePermissionUsedRecords(g_tokenIdA, "invalid_device"));
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::GetPermissionUsedRecords(request, result));
    ASSERT_EQ(static_cast<uint32_t>(1), result.bundleRecords.size());

    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::RemovePermissionUsedRecords(123, GetLocalDeviceUdid()));
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::GetPermissionUsedRecords(request, result));
    ASSERT_EQ(static_cast<uint32_t>(1), result.bundleRecords.size());
}

/**
 * @tc.name: RemovePermissionUsedRecords003
 * @tc.desc: RemovePermissionUsedRecords with valid tokenId and deviceID.
 * @tc.type: FUNC
 * @tc.require: issueI5P4IU
 */
HWTEST_F(PrivacyKitTest, RemovePermissionUsedRecords003, TestSize.Level1)
{
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, "ohos.permission.CAMERA", 1, 0));
    PermissionUsedRequest request;
    PermissionUsedResult result;
    std::vector<std::string> permissionList;
    BuildQueryRequest(g_tokenIdA, GetLocalDeviceUdid(), g_infoParmsA.bundleName, permissionList, request);

    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::RemovePermissionUsedRecords(g_tokenIdA, ""));
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::GetPermissionUsedRecords(request, result));
    ASSERT_EQ(static_cast<uint32_t>(0), result.bundleRecords.size());
}

/**
 * @tc.name: GetPermissionUsedRecords001
 * @tc.desc: cannot GetPermissionUsedRecords with invalid query time and flag.
 * @tc.type: FUNC
 * @tc.require: issueI5P4IU
 */
HWTEST_F(PrivacyKitTest, GetPermissionUsedRecords001, TestSize.Level1)
{
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, "ohos.permission.MICROPHONE", 1, 0));
    PermissionUsedRequest request;
    PermissionUsedResult result;
    std::vector<std::string> permissionList;
    BuildQueryRequest(g_tokenIdA, GetLocalDeviceUdid(), g_infoParmsA.bundleName, permissionList, request);
    request.beginTimeMillis = -1;
    request.endTimeMillis = -1;
    ASSERT_EQ(PrivacyError::ERR_PARAM_INVALID, PrivacyKit::GetPermissionUsedRecords(request, result));

    request.beginTimeMillis = 3;
    request.endTimeMillis = 1;
    ASSERT_EQ(PrivacyError::ERR_PARAM_INVALID, PrivacyKit::GetPermissionUsedRecords(request, result));

    request.flag = (PermissionUsageFlag)-1;
    ASSERT_EQ(PrivacyError::ERR_PARAM_INVALID, PrivacyKit::GetPermissionUsedRecords(request, result));
}

/**
 * @tc.name: GetPermissionUsedRecords002
 * @tc.desc: cannot GetPermissionUsedRecords with valid query time.
 * @tc.type: FUNC
 * @tc.require: issueI5P4IU
 */
HWTEST_F(PrivacyKitTest, GetPermissionUsedRecords002, TestSize.Level1)
{
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, "ohos.permission.MICROPHONE", 1, 0));
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, "ohos.permission.CAMERA", 1, 0));
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, "ohos.permission.READ_CALENDAR", 1, 0));

    PermissionUsedRequest request;
    PermissionUsedResult result;
    std::vector<std::string> permissionList;
    // query by tokenId
    BuildQueryRequest(g_tokenIdA, "", "", permissionList, request);
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::GetPermissionUsedRecords(request, result));
    ASSERT_EQ(static_cast<uint32_t>(1), result.bundleRecords.size());
    request.deviceId = GetLocalDeviceUdid();
    request.bundleName = g_infoParmsA.bundleName;
    CheckPermissionUsedResult(request, result, 3, 3, 0);

    // query by unmatched tokenId, deviceId and bundle Name
    BuildQueryRequest(123, GetLocalDeviceUdid(), g_infoParmsA.bundleName, permissionList, request);
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::GetPermissionUsedRecords(request, result));
    ASSERT_EQ(static_cast<uint32_t>(0), result.bundleRecords.size());

    // query by invalid permission Name
    permissionList.clear();
    permissionList.emplace_back("invalid permission");
    BuildQueryRequest(g_tokenIdA, GetLocalDeviceUdid(), g_infoParmsA.bundleName, permissionList, request);
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::GetPermissionUsedRecords(request, result));
    ASSERT_EQ(static_cast<uint32_t>(0), result.bundleRecords.size());
}

/**
 * @tc.name: GetPermissionUsedRecords003
 * @tc.desc: can GetPermissionUsedRecords with valid query time.
 * @tc.type: FUNC
 * @tc.require: issueI5P4IU
 */
HWTEST_F(PrivacyKitTest, GetPermissionUsedRecords003, TestSize.Level1)
{
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, "ohos.permission.MICROPHONE", 1, 0));
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, "ohos.permission.MICROPHONE", 1, 0));
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, "ohos.permission.MICROPHONE", 1, 0));
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, "ohos.permission.MICROPHONE", 1, 0));

    PermissionUsedRequest request;
    PermissionUsedResult result;
    std::vector<std::string> permissionList;
    BuildQueryRequest(g_tokenIdA, GetLocalDeviceUdid(), g_infoParmsA.bundleName, permissionList, request);
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::GetPermissionUsedRecords(request, result));
    ASSERT_EQ(static_cast<uint32_t>(1), result.bundleRecords.size());
    CheckPermissionUsedResult(request, result, 1, 4, 0);

    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, "ohos.permission.CAMERA", 1, 0));
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, "ohos.permission.READ_CALENDAR", 1, 0));
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, "ohos.permission.WRITE_CALENDAR", 1, 0));

    BuildQueryRequest(g_tokenIdA, GetLocalDeviceUdid(), g_infoParmsA.bundleName, permissionList, request);
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::GetPermissionUsedRecords(request, result));
    ASSERT_EQ(static_cast<uint32_t>(1), result.bundleRecords.size());
    CheckPermissionUsedResult(request, result, 4, 7, 0);
}

/**
 * @tc.name: GetPermissionUsedRecords004
 * @tc.desc: can GetPermissionUsedRecords with valid query time.
 * @tc.type: FUNC
 * @tc.require: issueI5P4IU
 */
HWTEST_F(PrivacyKitTest, GetPermissionUsedRecords004, TestSize.Level1)
{
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, "ohos.permission.CAMERA", 1, 0));
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, "ohos.permission.READ_CALENDAR", 1, 0));
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdB, "ohos.permission.CAMERA", 1, 0));
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdB, "ohos.permission.READ_CALENDAR", 1, 0));

    PermissionUsedRequest request;
    PermissionUsedResult result;
    std::vector<std::string> permissionList;
    BuildQueryRequest(0, GetLocalDeviceUdid(), "", permissionList, request);

    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::GetPermissionUsedRecords(request, result));
    if (result.bundleRecords.size() < static_cast<uint32_t>(2)) {
        ASSERT_TRUE(false);
    }
}

/**
 * @tc.name: GetPermissionUsedRecords005
 * @tc.desc: GetPermissionUsedRecords005 caller is normal app.
 * @tc.type: FUNC
 * @tc.require: issueI66BH3
 */
HWTEST_F(PrivacyKitTest, GetPermissionUsedRecords005, TestSize.Level1)
{
    AccessTokenIDEx tokenIdEx = {0};
    tokenIdEx = AccessTokenKit::AllocHapToken(g_normalInfoParms, g_policyPramsA);
    ASSERT_NE(INVALID_TOKENID, tokenIdEx.tokenIDEx);
    EXPECT_EQ(0, SetSelfTokenID(tokenIdEx.tokenIDEx));

    PermissionUsedRequest request;
    PermissionUsedResult result;
    std::vector<std::string> permissionList;
    // query by tokenId
    BuildQueryRequest(g_tokenIdA, "", "", permissionList, request);
    ASSERT_EQ(PrivacyError::ERR_NOT_SYSTEM_APP, PrivacyKit::GetPermissionUsedRecords(request, result));
}

/**
 * @tc.name: GetPermissionUsedRecordsAsync001
 * @tc.desc: cannot GetPermissionUsedRecordsAsync with invalid query time.
 * @tc.type: FUNC
 * @tc.require: issueI5P4IU
 */
HWTEST_F(PrivacyKitTest, GetPermissionUsedRecordsAsync001, TestSize.Level1)
{
    std::string permission = "ohos.permission.CAMERA";
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, permission, 1, 0));
    PermissionUsedRequest request;
    std::vector<std::string> permissionList;
    BuildQueryRequest(g_tokenIdA, GetLocalDeviceUdid(), "", permissionList, request);
    request.beginTimeMillis = -1;
    request.endTimeMillis = -1;
    OHOS::sptr<TestCallBack> callback(new TestCallBack());
    ASSERT_EQ(PrivacyError::ERR_PARAM_INVALID, PrivacyKit::GetPermissionUsedRecords(request, callback));
}

/**
 * @tc.name: GetPermissionUsedRecordsAsync002
 * @tc.desc: cannot GetPermissionUsedRecordsAsync with valid query time.
 * @tc.type: FUNC
 * @tc.require: issueI5P4IU
 */
HWTEST_F(PrivacyKitTest, GetPermissionUsedRecordsAsync002, TestSize.Level1)
{
    std::string permission = "ohos.permission.CAMERA";
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::AddPermissionUsedRecord(g_tokenIdA, permission, 1, 0));
    PermissionUsedRequest request;
    std::vector<std::string> permissionList;
    BuildQueryRequest(g_tokenIdA, GetLocalDeviceUdid(), "", permissionList, request);
    OHOS::sptr<TestCallBack> callback(new TestCallBack());
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::GetPermissionUsedRecords(request, callback));
}

class CbCustomizeTest1 : public PermActiveStatusCustomizedCbk {
public:
    explicit CbCustomizeTest1(const std::vector<std::string> &permList)
        : PermActiveStatusCustomizedCbk(permList)
    {
        GTEST_LOG_(INFO) << "CbCustomizeTest1 create";
    }

    ~CbCustomizeTest1()
    {}

    virtual void ActiveStatusChangeCallback(ActiveChangeResponse& result)
    {
        type_ = result.type;
        GTEST_LOG_(INFO) << "CbCustomizeTest1 ActiveChangeResponse";
        GTEST_LOG_(INFO) << "CbCustomizeTest1 tokenid " << result.tokenID;
        GTEST_LOG_(INFO) << "CbCustomizeTest1 permissionName " << result.permissionName;
        GTEST_LOG_(INFO) << "CbCustomizeTest1 deviceId " << result.deviceId;
        GTEST_LOG_(INFO) << "CbCustomizeTest1 type " << result.type;
    }

    ActiveChangeType type_ = PERM_INACTIVE;
};

class CbCustomizeTest2 : public PermActiveStatusCustomizedCbk {
public:
    explicit CbCustomizeTest2(const std::vector<std::string> &permList)
        : PermActiveStatusCustomizedCbk(permList)
    {
        GTEST_LOG_(INFO) << "CbCustomizeTest2 create";
    }

    ~CbCustomizeTest2()
    {}

    virtual void ActiveStatusChangeCallback(ActiveChangeResponse& result)
    {
        type_ = result.type;
        GTEST_LOG_(INFO) << "CbCustomizeTest2 ActiveChangeResponse";
        GTEST_LOG_(INFO) << "CbCustomizeTest2 tokenid " << result.tokenID;
        GTEST_LOG_(INFO) << "CbCustomizeTest2 permissionName " << result.permissionName;
        GTEST_LOG_(INFO) << "CbCustomizeTest2 deviceId " << result.deviceId;
        GTEST_LOG_(INFO) << "CbCustomizeTest2 type " << result.type;
    }

    ActiveChangeType type_;
};

/**
 * @tc.name: RegisterPermActiveStatusCallback001
 * @tc.desc: RegisterPermActiveStatusCallback with valid permission.
 * @tc.type: FUNC
 * @tc.require: issueI5NT1X issueI5P4IU issueI5P530
 */
HWTEST_F(PrivacyKitTest, RegisterPermActiveStatusCallback001, TestSize.Level1)
{
    std::vector<std::string> permList = {"ohos.permission.CAMERA"};

    auto callbackPtr = std::make_shared<CbCustomizeTest1>(permList);
    callbackPtr->type_ = PERM_INACTIVE;

    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::RegisterPermActiveStatusCallback(callbackPtr));

    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::StartUsingPermission(g_tokenIdE, "ohos.permission.CAMERA"));

    usleep(500000); // 500000us = 0.5s
    ASSERT_EQ(PERM_ACTIVE_IN_BACKGROUND, callbackPtr->type_);

    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::StopUsingPermission(g_tokenIdE, "ohos.permission.CAMERA"));

    usleep(500000); // 500000us = 0.5s
    ASSERT_EQ(PERM_INACTIVE, callbackPtr->type_);

    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::UnRegisterPermActiveStatusCallback(callbackPtr));
    callbackPtr->type_ = PERM_INACTIVE;

    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::StartUsingPermission(g_tokenIdE, "ohos.permission.CAMERA"));
    ASSERT_EQ(PERM_INACTIVE, callbackPtr->type_);

    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::StopUsingPermission(g_tokenIdE, "ohos.permission.CAMERA"));
    ASSERT_EQ(PERM_INACTIVE, callbackPtr->type_);
}

class CbCustomizeTest3 : public PermActiveStatusCustomizedCbk {
public:
    explicit CbCustomizeTest3(const std::vector<std::string> &permList)
        : PermActiveStatusCustomizedCbk(permList)
    {}
    ~CbCustomizeTest3()
    {}
    virtual void ActiveStatusChangeCallback(ActiveChangeResponse& result)
    {
        type_ = result.type;
    }
    ActiveChangeType type_ = PERM_INACTIVE;
};

class CbCustomizeTest4 : public StateCustomizedCbk {
public:
    CbCustomizeTest4()
    {}

    ~CbCustomizeTest4()
    {}

    virtual void StateChangeNotify(AccessTokenID tokenId, bool isShow)
    {}
};

/**
 * @tc.name: RegisterPermActiveStatusCallback002
 * @tc.desc: RegisterPermActiveStatusCallback with valid permission.
 * @tc.type: FUNC
 * @tc.require: issueI5NT1X issueI5P4IU issueI5P530
 */
HWTEST_F(PrivacyKitTest, RegisterPermActiveStatusCallback002, TestSize.Level1)
{
    std::vector<std::string> permList1 = {"ohos.permission.CAMERA"};
    auto callbackPtr1 = std::make_shared<CbCustomizeTest1>(permList1);
    callbackPtr1->type_ = PERM_INACTIVE;

    std::vector<std::string> permList2 = {"ohos.permission.MICROPHONE"};
    auto callbackPtr2 = std::make_shared<CbCustomizeTest2>(permList2);
    callbackPtr2->type_ = PERM_INACTIVE;

    std::vector<std::string> permList3 = {};
    auto callbackPtr3 = std::make_shared<CbCustomizeTest2>(permList3);
    callbackPtr3->type_ = PERM_INACTIVE;

    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::RegisterPermActiveStatusCallback(callbackPtr1));
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::RegisterPermActiveStatusCallback(callbackPtr2));
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::RegisterPermActiveStatusCallback(callbackPtr3));

    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::StartUsingPermission(g_tokenIdE, "ohos.permission.CAMERA"));

    usleep(500000); // 500000us = 0.5s
    ASSERT_EQ(PERM_ACTIVE_IN_BACKGROUND, callbackPtr1->type_);
    ASSERT_EQ(PERM_INACTIVE, callbackPtr2->type_);
    ASSERT_EQ(PERM_ACTIVE_IN_BACKGROUND, callbackPtr3->type_);

    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::StopUsingPermission(g_tokenIdE, "ohos.permission.CAMERA"));

    usleep(500000); // 500000us = 0.5s
    ASSERT_EQ(PERM_INACTIVE, callbackPtr1->type_);
    ASSERT_EQ(PERM_INACTIVE, callbackPtr3->type_);

    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::StartUsingPermission(g_tokenIdE, "ohos.permission.MICROPHONE"));

    usleep(500000); // 500000us = 0.5s
    ASSERT_EQ(PERM_INACTIVE, callbackPtr1->type_);
    ASSERT_EQ(PERM_ACTIVE_IN_BACKGROUND, callbackPtr2->type_);

    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::StopUsingPermission(g_tokenIdE, "ohos.permission.MICROPHONE"));

    usleep(500000); // 500000us = 0.5s
    ASSERT_EQ(PERM_INACTIVE, callbackPtr2->type_);

    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::UnRegisterPermActiveStatusCallback(callbackPtr1));
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::UnRegisterPermActiveStatusCallback(callbackPtr2));
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::UnRegisterPermActiveStatusCallback(callbackPtr3));
}

/**
 * @tc.name: RegisterPermActiveStatusCallback003
 * @tc.desc: Register callback with permList which contains 1025 permissions.
 * @tc.type: FUNC
 * @tc.require: issueI5NT1X
 */
HWTEST_F(PrivacyKitTest, RegisterPermActiveStatusCallback003, TestSize.Level1)
{
    std::vector<std::string> permList;
    for (int32_t i = 0; i < 1024; i++) { // 1024 is the limitation
        permList.emplace_back("ohos.permission.CAMERA");
    }
    auto callbackPtr1 = std::make_shared<CbCustomizeTest3>(permList);
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::RegisterPermActiveStatusCallback(callbackPtr1));
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::UnRegisterPermActiveStatusCallback(callbackPtr1));

    permList.emplace_back("ohos.permission.MICROPHONE");
    auto callbackPtr = std::make_shared<CbCustomizeTest3>(permList);
    ASSERT_EQ(PrivacyError::ERR_PARAM_INVALID, PrivacyKit::RegisterPermActiveStatusCallback(callbackPtr));
}

/**
 * @tc.name: RegisterPermActiveStatusCallback004
 * @tc.desc: Register callback 201 times.
 * @tc.type: FUNC
 * @tc.require: issueI5NT1X
 */
HWTEST_F(PrivacyKitTest, RegisterPermActiveStatusCallback004, TestSize.Level1)
{
    std::vector<std::string> permList = {"ohos.permission.CAMERA"};
    std::vector<std::shared_ptr<CbCustomizeTest3>> callbackList;

    for (int32_t i = 0; i <= 200; i++) { // 200 is the max size
        if (i == 200) { // 200 is the max size
            auto callbackPtr = std::make_shared<CbCustomizeTest3>(permList);
            ASSERT_EQ(PrivacyError::ERR_CALLBACKS_EXCEED_LIMITATION,
                PrivacyKit::RegisterPermActiveStatusCallback(callbackPtr));
            break;
        }
        auto callbackPtr = std::make_shared<CbCustomizeTest3>(permList);
        ASSERT_EQ(RET_NO_ERROR, PrivacyKit::RegisterPermActiveStatusCallback(callbackPtr));
        callbackList.emplace_back(callbackPtr);
    }
    for (int32_t i = 0; i < 200; i++) { // release 200 callback
        auto callbackPtr = callbackList[i];
        ASSERT_EQ(RET_NO_ERROR, PrivacyKit::UnRegisterPermActiveStatusCallback(callbackPtr));
    }
    callbackList.clear();
}

/**
 * @tc.name: RegisterPermActiveStatusCallback005
 * @tc.desc: RegisterPermActiveStatusCallback with valid permission.
 * @tc.type: FUNC
 * @tc.require: issueI5NT1X
 */
HWTEST_F(PrivacyKitTest, RegisterPermActiveStatusCallback005, TestSize.Level1)
{
    std::vector<std::string> permList = {"ohos.permission.INVALD"};

    auto callbackPtr = std::make_shared<CbCustomizeTest3>(permList);
    ASSERT_EQ(PrivacyError::ERR_PARAM_INVALID, PrivacyKit::RegisterPermActiveStatusCallback(callbackPtr));

    std::vector<std::string> permList1 = {"ohos.permission.INVALD", "ohos.permission.CAMERA"};
    auto callbackPtr1 = std::make_shared<CbCustomizeTest3>(permList1);
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::RegisterPermActiveStatusCallback(callbackPtr1));
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::UnRegisterPermActiveStatusCallback(callbackPtr1));
}

/**
 * @tc.name: RegisterPermActiveStatusCallback006
 * @tc.desc: UnRegisterPermActiveStatusCallback with invalid callback.
 * @tc.type: FUNC
 * @tc.require: issueI5NT1X
 */
HWTEST_F(PrivacyKitTest, RegisterPermActiveStatusCallback006, TestSize.Level1)
{
    std::vector<std::string> permList = {"ohos.permission.CAMERA"};
    auto callbackPtr = std::make_shared<CbCustomizeTest3>(permList);
    ASSERT_EQ(PrivacyError::ERR_CALLBACK_NOT_EXIST, PrivacyKit::UnRegisterPermActiveStatusCallback(callbackPtr));
}

/**
 * @tc.name: RegisterPermActiveStatusCallback007
 * @tc.desc: RegisterPermActiveStatusCallback with valid permission repeatedly.
 * @tc.type: FUNC
 * @tc.require: issueI5NT1X
 */
HWTEST_F(PrivacyKitTest, RegisterPermActiveStatusCallback007, TestSize.Level1)
{
    std::vector<std::string> permList = {"ohos.permission.CAMERA"};
    auto callbackPtr = std::make_shared<CbCustomizeTest3>(permList);
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::RegisterPermActiveStatusCallback(callbackPtr));
    ASSERT_EQ(PrivacyError::ERR_CALLBACK_ALREADY_EXIST, PrivacyKit::RegisterPermActiveStatusCallback(callbackPtr));
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::UnRegisterPermActiveStatusCallback(callbackPtr));
    ASSERT_EQ(PrivacyError::ERR_CALLBACK_NOT_EXIST, PrivacyKit::UnRegisterPermActiveStatusCallback(callbackPtr));
}

/**
 * @tc.name: RegisterPermActiveStatusCallback008
 * @tc.desc: register and startusing three permission.
 * @tc.type: FUNC
 * @tc.require: issueI5SZHG
 */
HWTEST_F(PrivacyKitTest, RegisterPermActiveStatusCallback008, TestSize.Level1)
{
    std::vector<std::string> permList = {
        "ohos.permission.CAMERA",
        "ohos.permission.MICROPHONE",
        "ohos.permission.LOCATION"
    };
    auto callbackPtr = std::make_shared<CbCustomizeTest3>(permList);
    int32_t res = PrivacyKit::RegisterPermActiveStatusCallback(callbackPtr);
    ASSERT_EQ(RET_NO_ERROR, res);
    res = PrivacyKit::StartUsingPermission(g_tokenIdE, "ohos.permission.CAMERA");
    ASSERT_EQ(RET_NO_ERROR, res);
    res = PrivacyKit::StartUsingPermission(g_tokenIdE, "ohos.permission.MICROPHONE");
    ASSERT_EQ(RET_NO_ERROR, res);
    res = PrivacyKit::StartUsingPermission(g_tokenIdE, "ohos.permission.LOCATION");
    ASSERT_EQ(RET_NO_ERROR, res);
    res = PrivacyKit::StopUsingPermission(g_tokenIdE, "ohos.permission.CAMERA");
    ASSERT_EQ(RET_NO_ERROR, res);
    res = PrivacyKit::StopUsingPermission(g_tokenIdE, "ohos.permission.MICROPHONE");
    ASSERT_EQ(RET_NO_ERROR, res);
    res = PrivacyKit::StopUsingPermission(g_tokenIdE, "ohos.permission.LOCATION");
    ASSERT_EQ(RET_NO_ERROR, res);
    res = PrivacyKit::UnRegisterPermActiveStatusCallback(callbackPtr);
    ASSERT_EQ(RET_NO_ERROR, res);
}

/**
 * @tc.name: RegisterPermActiveStatusCallback009
 * @tc.desc: PrivacyManagerClient::RegisterPermActiveStatusCallback function test
 * @tc.type: FUNC
 * @tc.require: issueI61A6M
 */
HWTEST_F(PrivacyKitTest, RegisterPermActiveStatusCallback009, TestSize.Level1)
{
    std::shared_ptr<PermActiveStatusCustomizedCbk> callback = nullptr;
    ASSERT_EQ(nullptr, callback);
    PrivacyManagerClient::GetInstance().RegisterPermActiveStatusCallback(callback); // callback is null
}

/**
 * @tc.name: RegisterPermActiveStatusCallback010
 * @tc.desc: RegisterPermActiveStatusCallback caller is normal app.
 * @tc.type: FUNC
 * @tc.require: issueI66BH3
 */
HWTEST_F(PrivacyKitTest, RegisterPermActiveStatusCallback010, TestSize.Level1)
{
    AccessTokenIDEx tokenIdEx = {0};
    tokenIdEx = AccessTokenKit::AllocHapToken(g_normalInfoParms, g_policyPramsA);
    ASSERT_NE(INVALID_TOKENID, tokenIdEx.tokenIDEx);
    EXPECT_EQ(0, SetSelfTokenID(tokenIdEx.tokenIDEx));

    std::vector<std::string> permList1 = {"ohos.permission.CAMERA"};
    auto callbackPtr = std::make_shared<CbCustomizeTest3>(permList1);
    ASSERT_EQ(PrivacyError::ERR_NOT_SYSTEM_APP, PrivacyKit::RegisterPermActiveStatusCallback(callbackPtr));
}

/**
 * @tc.name: RegisterPermActiveStatusCallback011
 * @tc.desc: UnRegisterPermActiveStatusCallback caller is normal app.
 * @tc.type: FUNC
 * @tc.require: issueI66BH3
 */
HWTEST_F(PrivacyKitTest, RegisterPermActiveStatusCallback011, TestSize.Level1)
{
    std::vector<std::string> permList1 = {"ohos.permission.CAMERA"};
    auto callbackPtr1 = std::make_shared<CbCustomizeTest3>(permList1);
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::RegisterPermActiveStatusCallback(callbackPtr1));

    AccessTokenIDEx tokenIdEx = {0};
    tokenIdEx = AccessTokenKit::AllocHapToken(g_normalInfoParms, g_policyPramsA);
    ASSERT_NE(INVALID_TOKENID, tokenIdEx.tokenIDEx);
    EXPECT_EQ(0, SetSelfTokenID(tokenIdEx.tokenIDEx));
    ASSERT_EQ(PrivacyError::ERR_NOT_SYSTEM_APP, PrivacyKit::UnRegisterPermActiveStatusCallback(callbackPtr1));

    EXPECT_EQ(0, SetSelfTokenID(g_selfTokenId));
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::UnRegisterPermActiveStatusCallback(callbackPtr1));
}

/**
 * @tc.name: IsAllowedUsingPermission001
 * @tc.desc: IsAllowedUsingPermission with invalid tokenId or permission.
 * @tc.type: FUNC
 * @tc.require: issueI5RWX3 issueI5RWX8
 */
HWTEST_F(PrivacyKitTest, IsAllowedUsingPermission001, TestSize.Level1)
{
    std::string permissionName = "ohos.permission.CAMERA";
    ASSERT_EQ(false, PrivacyKit::IsAllowedUsingPermission(g_tokenIdE, permissionName));
    ASSERT_EQ(false, PrivacyKit::IsAllowedUsingPermission(0, permissionName));
    ASSERT_EQ(false, PrivacyKit::IsAllowedUsingPermission(g_tokenIdE, ""));
}

/**
 * @tc.name: IsAllowedUsingPermission002
 * @tc.desc: IsAllowedUsingPermission with no permission.
 * @tc.type: FUNC
 * @tc.require: issueI5RWX3 issueI5RWX8
 */
HWTEST_F(PrivacyKitTest, IsAllowedUsingPermission002, TestSize.Level1)
{
    EXPECT_EQ(0, SetSelfTokenID(g_selfTokenId));
    std::string permissionName = "ohos.permission.CAMERA";
    ASSERT_EQ(false, PrivacyKit::IsAllowedUsingPermission(g_tokenIdE, permissionName));
}
/**
 * @tc.name: StartUsingPermission001
 * @tc.desc: StartUsingPermission with invalid tokenId or permission.
 * @tc.type: FUNC
 * @tc.require: issueI5NT1X issueI5P4IU issueI5P530
 */
HWTEST_F(PrivacyKitTest, StartUsingPermission001, TestSize.Level1)
{
    ASSERT_EQ(PrivacyError::ERR_PARAM_INVALID, PrivacyKit::StartUsingPermission(0, "ohos.permission.CAMERA"));
    ASSERT_EQ(PrivacyError::ERR_PARAM_INVALID, PrivacyKit::StartUsingPermission(0, "permissionName"));
}

/**
 * @tc.name: StartUsingPermission002
 * @tc.desc: StartUsingPermission is called twice with same permission.
 * @tc.type: FUNC
 * @tc.require: issueI5NT1X issueI5P4IU issueI5P530
 */
HWTEST_F(PrivacyKitTest, StartUsingPermission002, TestSize.Level1)
{
    std::string permissionName = "ohos.permission.CAMERA";
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::StartUsingPermission(g_tokenIdE, permissionName));
    ASSERT_EQ(PrivacyError::ERR_PERMISSION_ALREADY_START_USING,
        PrivacyKit::StartUsingPermission(g_tokenIdE, permissionName));
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::StopUsingPermission(g_tokenIdE, permissionName));
}

/**
 * @tc.name: StopUsingPermission003
 * @tc.desc: Add record when StopUsingPermission is called.
 * @tc.type: FUNC
 * @tc.require: issueI5NT1X issueI5P4IU issueI5P530
 */
HWTEST_F(PrivacyKitTest, StartUsingPermission003, TestSize.Level1)
{
    std::string permissionName = "ohos.permission.CAMERA";
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::StartUsingPermission(g_tokenIdE, permissionName));

    usleep(500000); // 500000us = 0.5s
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::StopUsingPermission(g_tokenIdE, permissionName));

    PermissionUsedRequest request;
    PermissionUsedResult result;
    std::vector<std::string> permissionList;
    BuildQueryRequest(g_tokenIdE, GetLocalDeviceUdid(), g_infoParmsE.bundleName, permissionList, request);
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::GetPermissionUsedRecords(request, result));
    ASSERT_EQ(static_cast<uint32_t>(1), result.bundleRecords.size());
    ASSERT_EQ(g_tokenIdE, result.bundleRecords[0].tokenId);
    ASSERT_EQ(g_infoParmsE.bundleName, result.bundleRecords[0].bundleName);
    ASSERT_EQ(static_cast<uint32_t>(1), result.bundleRecords[0].permissionRecords.size());
    ASSERT_EQ(2, result.bundleRecords[0].permissionRecords[0].accessCount);
}

/**
 * @tc.name: StartUsingPermission004
 * @tc.desc: StartUsingPermission basic functional verification
 * @tc.type: FUNC
 * @tc.require: issueI5NT1X issueI5P4IU issueI5P530
 */
HWTEST_F(PrivacyKitTest, StartUsingPermission004, TestSize.Level1)
{
    std::string permissionName = "ohos.permission.CAMERA";
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::StartUsingPermission(g_tokenIdE, permissionName));
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::StopUsingPermission(g_tokenIdE, permissionName));
}

/**
 * @tc.name: StartUsingPermission005
 * @tc.desc: StartUsingPermission with invalid tokenId or permission.
 * @tc.type: FUNC
 * @tc.require: issueI5NT1X issueI5P4IU issueI5P530
 */
HWTEST_F(PrivacyKitTest, StartUsingPermission005, TestSize.Level1)
{
    std::string permissionName = "ohos.permission.UtTestInvalidPermission";
    ASSERT_EQ(PrivacyError::ERR_PERMISSION_NOT_EXIST, PrivacyKit::StartUsingPermission(g_tokenIdE, permissionName));
    ASSERT_EQ(PrivacyError::ERR_PARAM_INVALID, PrivacyKit::StartUsingPermission(0, "ohos.permission.CAMERA"));
    ASSERT_EQ(PrivacyError::ERR_TOKENID_NOT_EXIST, PrivacyKit::StartUsingPermission(1, "ohos.permission.CAMERA"));
}

/**
 * @tc.name: StartUsingPermission006
 * @tc.desc: StartUsingPermission with invalid tokenId or permission or callback.
 * @tc.type: FUNC
 * @tc.require: issueI5RWX5 issueI5RWX3 issueI5RWXA
 */
HWTEST_F(PrivacyKitTest, StartUsingPermission006, TestSize.Level1)
{
    auto callbackPtr = std::make_shared<CbCustomizeTest4>();
    ASSERT_EQ(PrivacyError::ERR_PARAM_INVALID,
        PrivacyKit::StartUsingPermission(0, "ohos.permission.CAMERA", callbackPtr));
    ASSERT_EQ(PrivacyError::ERR_PARAM_INVALID,
        PrivacyKit::StartUsingPermission(g_tokenIdE, "", callbackPtr));
    ASSERT_EQ(PrivacyError::ERR_PARAM_INVALID,
        PrivacyKit::StartUsingPermission(g_tokenIdE, "ohos.permission.CAMERA", nullptr));
    ASSERT_EQ(PrivacyError::ERR_PARAM_INVALID,
        PrivacyKit::StartUsingPermission(g_tokenIdE, "permissionName", callbackPtr));
}

/**
 * @tc.name: StartUsingPermission007
 * @tc.desc: StartUsingPermission is called twice with same callback.
 * @tc.type: FUNC
 * @tc.require: issueI5RWX5 issueI5RWX3 issueI5RWXA
 */
HWTEST_F(PrivacyKitTest, StartUsingPermission007, TestSize.Level1)
{
    std::string permissionName = "ohos.permission.CAMERA";
    auto callbackPtr = std::make_shared<CbCustomizeTest4>();
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::StartUsingPermission(g_tokenIdE, permissionName, callbackPtr));
    ASSERT_EQ(PrivacyError::ERR_CALLBACK_ALREADY_EXIST,
        PrivacyKit::StartUsingPermission(g_tokenIdE, permissionName, callbackPtr));
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::StopUsingPermission(g_tokenIdE, permissionName));
}

/**
 * @tc.name: StopUsingPermission008
 * @tc.desc: Add record when StopUsingPermission is called.
 * @tc.type: FUNC
 * @tc.require: issueI5RWX5 issueI5RWX3 issueI5RWXA
 */
HWTEST_F(PrivacyKitTest, StopUsingPermission008, TestSize.Level1)
{
    std::string permissionName = "ohos.permission.CAMERA";
    auto callbackPtr = std::make_shared<CbCustomizeTest4>();
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::StartUsingPermission(g_tokenIdE, permissionName, callbackPtr));

    usleep(500000); // 500000us = 0.5s
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::StopUsingPermission(g_tokenIdE, permissionName));

    PermissionUsedRequest request;
    PermissionUsedResult result;
    std::vector<std::string> permissionList;
    BuildQueryRequest(g_tokenIdE, GetLocalDeviceUdid(), g_infoParmsE.bundleName, permissionList, request);
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::GetPermissionUsedRecords(request, result));
    ASSERT_EQ(static_cast<uint32_t>(1), result.bundleRecords.size());
    ASSERT_EQ(g_tokenIdE, result.bundleRecords[0].tokenId);
    ASSERT_EQ(g_infoParmsE.bundleName, result.bundleRecords[0].bundleName);
    ASSERT_EQ(static_cast<uint32_t>(1), result.bundleRecords[0].permissionRecords.size());
    ASSERT_EQ(2, result.bundleRecords[0].permissionRecords[0].accessCount);
}

/**
 * @tc.name: StartUsingPermission009
 * @tc.desc: StartUsingPermission basic functional verification
 * @tc.type: FUNC
 * @tc.require: issueI5RWX5 issueI5RWX3 issueI5RWXA
 */
HWTEST_F(PrivacyKitTest, StartUsingPermission009, TestSize.Level1)
{
    std::string permissionName = "ohos.permission.CAMERA";
    auto callbackPtr = std::make_shared<CbCustomizeTest4>();
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::StartUsingPermission(g_tokenIdE, permissionName, callbackPtr));
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::StopUsingPermission(g_tokenIdE, permissionName));
}

/**
 * @tc.name: StartUsingPermission010
 * @tc.desc: StartUsingPermission caller is normal app.
 * @tc.type: FUNC
 * @tc.require: issueI66BH3
 */
HWTEST_F(PrivacyKitTest, StartUsingPermission010, TestSize.Level1)
{
    g_policyPramsA.permStateList.emplace_back(g_infoManagerTestStateC);
    AccessTokenIDEx tokenIdEx = {0};
    tokenIdEx = AccessTokenKit::AllocHapToken(g_normalInfoParms, g_policyPramsA);
    ASSERT_NE(INVALID_TOKENID, tokenIdEx.tokenIDEx);
    EXPECT_EQ(0, SetSelfTokenID(tokenIdEx.tokenIDEx));

    std::string permissionName = "ohos.permission.CAMERA";
    ASSERT_EQ(PrivacyError::ERR_NOT_SYSTEM_APP, PrivacyKit::StartUsingPermission(g_tokenIdE, permissionName));
}

/**
 * @tc.name: StopUsingPermission001
 * @tc.desc: StopUsingPermission with invalid tokenId or permission.
 * @tc.type: FUNC
 * @tc.require: issueI5NT1X issueI5P4IU issueI5P530
 */
HWTEST_F(PrivacyKitTest, StopUsingPermission001, TestSize.Level1)
{
    ASSERT_EQ(PrivacyError::ERR_PARAM_INVALID, PrivacyKit::StopUsingPermission(0, "ohos.permission.CAMERA"));
    ASSERT_EQ(PrivacyError::ERR_PARAM_INVALID, PrivacyKit::StopUsingPermission(0, "permissionName"));
}

/**
 * @tc.name: StopUsingPermission002
 * @tc.desc: StopUsingPermission cancel permissions that you haven't started using
 * @tc.type: FUNC
 * @tc.require: issueI5NT1X issueI5P4IU issueI5P530
 */
HWTEST_F(PrivacyKitTest, StopUsingPermission002, TestSize.Level1)
{
    std::string permissionName = "ohos.permission.CAMERA";
    ASSERT_EQ(
        PrivacyError::ERR_PERMISSION_NOT_START_USING, PrivacyKit::StopUsingPermission(g_tokenIdE, permissionName));
}

/**
 * @tc.name: StopUsingPermission003
 * @tc.desc: StopUsingPermission invalid tokenid, permission
 * @tc.type: FUNC
 * @tc.require: issueI5NT1X issueI5P4IU issueI5P530
 */
HWTEST_F(PrivacyKitTest, StopUsingPermission003, TestSize.Level1)
{
    std::string permissionName = "ohos.permission.CAMERA";
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::StartUsingPermission(g_tokenIdE, permissionName));
    ASSERT_EQ(PrivacyError::ERR_PERMISSION_NOT_EXIST,
        PrivacyKit::StopUsingPermission(g_tokenIdE, "ohos.permission.uttestpermission"));
    ASSERT_EQ(PrivacyError::ERR_PARAM_INVALID, PrivacyKit::StopUsingPermission(0, permissionName));
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::StopUsingPermission(g_tokenIdE, permissionName));
}

/**
 * @tc.name: StopUsingPermission004
 * @tc.desc: StopUsingPermission stop a use repeatedly
 * @tc.type: FUNC
 * @tc.require: issueI5NT1X issueI5P4IU issueI5P530
 */
HWTEST_F(PrivacyKitTest, StopUsingPermission004, TestSize.Level1)
{
    std::string permissionName = "ohos.permission.CAMERA";
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::StartUsingPermission(g_tokenIdE, permissionName));
    ASSERT_EQ(RET_NO_ERROR, PrivacyKit::StopUsingPermission(g_tokenIdE, permissionName));
    ASSERT_EQ(
        PrivacyError::ERR_PERMISSION_NOT_START_USING, PrivacyKit::StopUsingPermission(g_tokenIdE, permissionName));
}

/**
 * @tc.name: StopUsingPermission005
 * @tc.desc: stop use whith native token
 * @tc.type: FUNC
 * @tc.require: issueI5SZHG
 */
HWTEST_F(PrivacyKitTest, StopUsingPermission005, TestSize.Level1)
{
    AccessTokenID tokenId = AccessTokenKit::GetNativeTokenId("privacy_service");
    ASSERT_EQ(PrivacyError::ERR_TOKENID_NOT_EXIST, PrivacyKit::StopUsingPermission(tokenId, "ohos.permission.CAMERA"));
}

/**
 * @tc.name: StopUsingPermission006
 * @tc.desc: StopUsingPermission caller is normal app.
 * @tc.type: FUNC
 * @tc.require: issueI66BH3
 */
HWTEST_F(PrivacyKitTest, StopUsingPermission006, TestSize.Level1)
{
    AccessTokenIDEx tokenIdEx = {0};
    tokenIdEx = AccessTokenKit::AllocHapToken(g_normalInfoParms, g_policyPramsA);
    ASSERT_NE(INVALID_TOKENID, tokenIdEx.tokenIDEx);
    EXPECT_EQ(0, SetSelfTokenID(tokenIdEx.tokenIDEx));

    std::string permissionName = "ohos.permission.CAMERA";
    ASSERT_EQ(PrivacyError::ERR_NOT_SYSTEM_APP, PrivacyKit::StopUsingPermission(g_tokenIdE, permissionName));
}

class TestCallBack1 : public StateChangeCallbackStub {
public:
    TestCallBack1() = default;
    virtual ~TestCallBack1() = default;

    void StateChangeNotify(AccessTokenID tokenId, bool isShowing)
    {
        GTEST_LOG_(INFO) << "StateChangeNotify,  tokenId is " << tokenId << ", isShowing is " << isShowing;
    }
};

/**
 * @tc.name: OnRemoteRequest001
 * @tc.desc: StateChangeCallbackStub::OnRemoteRequest function test
 * @tc.type: FUNC
 * @tc.require: issueI61A6M
 */
HWTEST_F(PrivacyKitTest, OnRemoteRequest001, TestSize.Level1)
{
    AccessTokenID tokenId = 123; // 123 is random input
    bool isShowing = false;

    TestCallBack1 callback;
    OHOS::MessageParcel data;
    ASSERT_EQ(true, data.WriteInterfaceToken(IStateChangeCallback::GetDescriptor()));
    ASSERT_EQ(true, data.WriteUint32(tokenId));
    ASSERT_EQ(true, data.WriteBool(isShowing));

    OHOS::MessageParcel reply;
    OHOS::MessageOption option(OHOS::MessageOption::TF_SYNC);
    ASSERT_EQ(0, callback.OnRemoteRequest(static_cast<uint32_t>(IStateChangeCallback::STATE_CHANGE_CALLBACK),
        data, reply, option)); // descriptor true + msgCode true

    ASSERT_EQ(true, data.WriteInterfaceToken(IStateChangeCallback::GetDescriptor()));
    ASSERT_EQ(true, data.WriteUint32(tokenId));
    ASSERT_EQ(true, data.WriteBool(isShowing));
    uint32_t code = 10;
    ASSERT_NE(0, callback.OnRemoteRequest(code, data, reply, option)); // descriptor true + msgCode false

    std::string descriptor = "I don't know";
    data.WriteInterfaceToken(OHOS::Str8ToStr16(descriptor));
    ASSERT_EQ(true, data.WriteUint32(tokenId));
    ASSERT_EQ(true, data.WriteBool(isShowing));
    ASSERT_NE(0, callback.OnRemoteRequest(static_cast<uint32_t>(IStateChangeCallback::STATE_CHANGE_CALLBACK),
        data, reply, option)); // descriptor flase + msgCode true
}

class TestCallBack2 : public OnPermissionUsedRecordCallbackStub {
public:
    TestCallBack2() = default;
    virtual ~TestCallBack2() = default;

    void OnQueried(OHOS::ErrCode code, PermissionUsedResult& result)
    {
        GTEST_LOG_(INFO) << "OnQueried,  code is " << code;
    }
};

/**
 * @tc.name: OnRemoteRequest002
 * @tc.desc: OnPermissionUsedRecordCallbackStub::OnRemoteRequest function test
 * @tc.type: FUNC
 * @tc.require: issueI61A6M
 */
HWTEST_F(PrivacyKitTest, OnRemoteRequest002, TestSize.Level1)
{
    g_permissionUsedRecord.accessRecords.emplace_back(g_usedRecordDetail);
    g_bundleUsedRecord.permissionRecords.emplace_back(g_permissionUsedRecord);

    int32_t errCode = 123; // 123 is random input
    PermissionUsedResultParcel resultParcel;
    resultParcel.result = {
        .beginTimeMillis = 0L,
        .endTimeMillis = 0L,
    };
    resultParcel.result.bundleRecords.emplace_back(g_bundleUsedRecord);

    TestCallBack2 callback;
    OHOS::MessageParcel data;
    std::string descriptor = "I don't know";
    data.WriteInterfaceToken(OHOS::Str8ToStr16(descriptor));
    ASSERT_EQ(true, data.ReadInt32(errCode));
    ASSERT_EQ(true, data.WriteParcelable(&resultParcel));

    OHOS::MessageParcel reply;
    OHOS::MessageOption option(OHOS::MessageOption::TF_SYNC);
    ASSERT_NE(0, callback.OnRemoteRequest(static_cast<uint32_t>(PrivacyPermissionRecordInterfaceCode::ON_QUERIED),
        data, reply, option)); // descriptor false

    ASSERT_EQ(true, data.WriteInterfaceToken(OnPermissionUsedRecordCallback::GetDescriptor()));
    ASSERT_EQ(true, data.ReadInt32(errCode));
    ASSERT_EQ(true, data.WriteParcelable(&resultParcel));
    uint32_t code = 10;
    ASSERT_NE(0, callback.OnRemoteRequest(code, data, reply, option)); // descriptor true + msgCode false

    ASSERT_EQ(true, data.WriteInterfaceToken(OnPermissionUsedRecordCallback::GetDescriptor()));
    ASSERT_EQ(true, data.ReadInt32(errCode));
    ASSERT_EQ(true, data.WriteParcelable(&resultParcel));
    ASSERT_NE(0, callback.OnRemoteRequest(static_cast<uint32_t>(PrivacyPermissionRecordInterfaceCode::ON_QUERIED),
        data, reply, option)); // descriptor flase + msgCode true + error != 0
}

class TestCallBack3 : public PermActiveStatusChangeCallbackStub {
public:
    TestCallBack3() = default;
    virtual ~TestCallBack3() = default;

    void ActiveStatusChangeCallback(ActiveChangeResponse& result)
    {
        GTEST_LOG_(INFO) << "ActiveStatusChangeCallback,  result is " << result.type;
    }
};

/**
 * @tc.name: OnRemoteRequest003
 * @tc.desc: PermActiveStatusChangeCallbackStub::OnRemoteRequest function test
 * @tc.type: FUNC
 * @tc.require: issueI61A6M
 */
HWTEST_F(PrivacyKitTest, OnRemoteRequest003, TestSize.Level1)
{
    ActiveChangeResponse response = {
        .tokenID = 123,
        .permissionName = "ohos.permission.CAMERA",
        .deviceId = "I don't know",
        .type = ActiveChangeType::PERM_INACTIVE
    };

    ActiveChangeResponseParcel responseParcel;
    responseParcel.changeResponse = response;

    TestCallBack3 callback;
    OHOS::MessageParcel data;
    std::string descriptor = "I don't know";
    data.WriteInterfaceToken(OHOS::Str8ToStr16(descriptor));
    ASSERT_EQ(true, data.WriteParcelable(&responseParcel));

    OHOS::MessageParcel reply;
    OHOS::MessageOption option(OHOS::MessageOption::TF_SYNC);
    ASSERT_NE(0, callback.OnRemoteRequest(static_cast<uint32_t>(
        PrivacyActiveChangeInterfaceCode::PERM_ACTIVE_STATUS_CHANGE), data, reply, option)); // descriptor false

    ASSERT_EQ(true, data.WriteInterfaceToken(IPermActiveStatusCallback::GetDescriptor()));
    ASSERT_EQ(true, data.WriteParcelable(&responseParcel));
    uint32_t code = 10;
    ASSERT_NE(0, callback.OnRemoteRequest(code, data, reply, option)); // descriptor true + msgCode false
}

/**
 * @tc.name: ActiveStatusChangeCallback001
 * @tc.desc: PermActiveStatusChangeCallback::ActiveStatusChangeCallback function test
 * @tc.type: FUNC
 * @tc.require: issueI61A6M
 */
HWTEST_F(PrivacyKitTest, ActiveStatusChangeCallback001, TestSize.Level1)
{
    ActiveChangeResponse response = {
        .tokenID = 123,
        .permissionName = "ohos.permission.CAMERA",
        .deviceId = "I don't know",
        .type = ActiveChangeType::PERM_INACTIVE
    };
    std::vector<std::string> permList = {"ohos.permission.CAMERA"};
    std::shared_ptr<PermActiveStatusCustomizedCbk> callbackPtr = std::make_shared<CbCustomizeTest1>(permList);
    OHOS::sptr<PermActiveStatusChangeCallback> callback = new (
        std::nothrow) PermActiveStatusChangeCallback(callbackPtr);
    ASSERT_NE(nullptr, callback);
    callback->ActiveStatusChangeCallback(response); // customizedCallback_ is null
}

/**
 * @tc.name: StateChangeNotify001
 * @tc.desc: StateChangeCallback::StateChangeNotify function test
 * @tc.type: FUNC
 * @tc.require: issueI61A6M
 */
HWTEST_F(PrivacyKitTest, StateChangeNotify001, TestSize.Level1)
{
    AccessTokenID tokenId = 123; // 123 is random input
    bool isShowing = false;
    std::shared_ptr<StateCustomizedCbk> callbackPtr = std::make_shared<CbCustomizeTest4>();
    OHOS::sptr<StateChangeCallback> callback = new (std::nothrow) StateChangeCallback(callbackPtr);
    ASSERT_NE(nullptr, callback);
    callback->StateChangeNotify(tokenId, isShowing); // customizedCallback_ is null
}

/**
 * @tc.name: InitProxy001
 * @tc.desc: PrivacyManagerClient::InitProxy function test
 * @tc.type: FUNC
 * @tc.require: issueI61A6M
 */
HWTEST_F(PrivacyKitTest, InitProxy001, TestSize.Level1)
{
    ASSERT_NE(nullptr, PrivacyManagerClient::GetInstance().proxy_);
    OHOS::sptr<IPrivacyManager> proxy = PrivacyManagerClient::GetInstance().proxy_; // backup
    PrivacyManagerClient::GetInstance().proxy_ = nullptr;
    ASSERT_EQ(nullptr, PrivacyManagerClient::GetInstance().proxy_);
    PrivacyManagerClient::GetInstance().InitProxy(); // proxy_ is null
    PrivacyManagerClient::GetInstance().proxy_ = proxy; // recovery
}

/**
 * @tc.name: StartUsingPermission008
 * @tc.desc: PrivacyKit:: function test input invalid
 * @tc.type: FUNC
 * @tc.require: issueI61A6M
 */
HWTEST_F(PrivacyKitTest, StartUsingPermission008, TestSize.Level1)
{
    AccessTokenID tokenId = 0;
    std::string permissionName;
    ASSERT_EQ(PrivacyError::ERR_PARAM_INVALID, PrivacyKit::StartUsingPermission(tokenId, permissionName));
    ASSERT_EQ(PrivacyError::ERR_PARAM_INVALID, PrivacyKit::StopUsingPermission(tokenId, permissionName));
    ASSERT_EQ(PrivacyError::ERR_PARAM_INVALID, PrivacyKit::RemovePermissionUsedRecords(tokenId, permissionName));
    ASSERT_EQ(false, PrivacyKit::IsAllowedUsingPermission(tokenId, permissionName));
}

#ifdef SECURITY_COMPONENT_ENHANCE_ENABLE
/**
 * @tc.name: StartUsingPermission008
 * @tc.desc: PrivacyKit:: function test register enhance data
 * @tc.type: FUNC
 * @tc.require: issueI7MXZ
 */
HWTEST_F(PrivacyKitTest, RegisterSecCompEnhance001, TestSize.Level1)
{
    SecCompEnhanceData data;
    data.callback = nullptr;
    data.pid = 1;
    data.challenge = 0;
    ASSERT_EQ(PrivacyError::ERR_WRITE_PARCEL_FAILED, PrivacyKit::RegisterSecCompEnhance(data));

    // StateChangeCallback is not the real callback of SecCompEnhance, but it does not effect the final result.
    auto callbackPtr = std::make_shared<CbCustomizeTest4>();
    data.callback = new (std::nothrow) StateChangeCallback(callbackPtr);
    ASSERT_EQ(RET_SUCCESS, PrivacyKit::RegisterSecCompEnhance(data));
}

/**
 * @tc.name: DepositSecCompEnhance001
 * @tc.desc: PrivacyKit:: function test Deposit and Recover
 * @tc.type: FUNC
 * @tc.require: issueI7MXZ
 */
HWTEST_F(PrivacyKitTest, DepositSecCompEnhance001, TestSize.Level1)
{
    AccessTokenID secCompId = AccessTokenKit::GetNativeTokenId("security_component_service");
    EXPECT_EQ(0, SetSelfTokenID(secCompId));

    SecCompEnhanceData data;
    data.callback = nullptr;
    data.pid = 1;
    data.challenge = 0;

    SecCompEnhanceData data1;
    // StateChangeCallback is not the real callback of SecCompEnhance, but it does not effect the final result.
    auto callbackPtr = std::make_shared<CbCustomizeTest4>();
    data1.callback = new (std::nothrow) StateChangeCallback(callbackPtr);
    data1.pid = 2;
    data1.challenge = 0;

    std::vector<SecCompEnhanceData> enhanceList;
    enhanceList.emplace_back(data);
    enhanceList.emplace_back(data1);

    ASSERT_EQ(RET_SUCCESS, PrivacyKit::DepositSecCompEnhance(enhanceList));

    std::vector<SecCompEnhanceData> res;
    ASSERT_EQ(RET_SUCCESS, PrivacyKit::RecoverSecCompEnhance(res));

    bool isPidExist = std::any_of(res.begin(), res.end(), [](const SecCompEnhanceData & data) {
        return data.pid == 2;
    });
    ASSERT_TRUE(isPidExist);
    EXPECT_EQ(0, SetSelfTokenID(g_selfTokenId));
}
#endif
