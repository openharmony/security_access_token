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

#include <cstdint>
#include <gtest/gtest.h>
#include <string>

#include "access_token.h"
#include "accesstoken_kit.h"
#include "accesstoken_log.h"
#include "active_change_response_info.h"
#include "audio_manager_adapter.h"
#include "constant.h"
#include "data_translator.h"
#include "permission_record.h"
#define private public
#include "active_status_callback_manager.h"
#include "libraryloader.h"
#include "permission_record_manager.h"
#include "permission_used_record_db.h"
#include "privacy_manager_service.h"
#undef private
#include "parameter.h"
#include "perm_active_status_change_callback_stub.h"
#include "privacy_error.h"
#include "privacy_field_const.h"
#include "privacy_kit.h"
#include "state_change_callback.h"
#include "time_util.h"
#include "token_setproc.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
namespace Security {
namespace AccessToken {
namespace {
static int32_t PID = -1;
static int32_t TEST_PID_1 = 1;
static int32_t TEST_PID_2 = 2;
static int32_t TEST_PID_3 = 3;
static AccessTokenID g_selfTokenId = 0;
static AccessTokenID g_nativeToken = 0;
static bool g_isMicEdmMute = false;
static bool g_isMicMixMute = false;
static constexpr uint32_t MAX_CALLBACK_SIZE = 1024;
static constexpr int32_t RANDOM_TOKENID = 123;
static constexpr int32_t FIRST_INDEX = 0;
static const int32_t NORMAL_TYPE_ADD_VALUE = 1;
static const int32_t PICKER_TYPE_ADD_VALUE = 2;
static const int32_t SEC_COMPONENT_TYPE_ADD_VALUE = 4;
static const int32_t VALUE_MAX_LEN = 32;
static const char* EDM_MIC_MUTE_KEY = "persist.edm.mic_disable";
static PermissionStateFull g_testState1 = {
    .permissionName = "ohos.permission.CAMERA",
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
    .permissionName = "ohos.permission.MANAGE_AUDIO_CONFIG",
    .isGeneral = true,
    .resDeviceID = {"local"},
    .grantStatus = {PermissionState::PERMISSION_GRANTED},
    .grantFlags = {1}
};

static HapPolicyParams g_PolicyPrams1 = {
    .apl = APL_NORMAL,
    .domain = "test.domain.A",
    .permList = {},
    .permStateList = {g_testState1, g_testState2, g_testState3}
};

static HapInfoParams g_InfoParms1 = {
    .userID = 1,
    .bundleName = "ohos.privacy_test.bundleA",
    .instIndex = 0,
    .appIDDesc = "privacy_test.bundleA"
};

static HapPolicyParams g_PolicyPrams2 = {
    .apl = APL_NORMAL,
    .domain = "test.domain.B",
    .permList = {},
    .permStateList = {g_testState1}
};

static HapInfoParams g_InfoParms2 = {
    .userID = 1,
    .bundleName = "ohos.privacy_test.bundleB",
    .instIndex = 0,
    .appIDDesc = "privacy_test.bundleB"
};
}
class PermissionRecordManagerTest : public testing::Test {
public:
    static void SetUpTestCase();

    static void TearDownTestCase();

    void SetUp();

    void TearDown();

    std::shared_ptr<PrivacyAppStateObserver> appStateObserver_ = nullptr;
};

void PermissionRecordManagerTest::SetUpTestCase()
{
    DelayedSingleton<PrivacyManagerService>::GetInstance()->Initialize();
    PermissionRecordManager::GetInstance().Init();

    g_selfTokenId = GetSelfTokenID();
    g_nativeToken = AccessTokenKit::GetNativeTokenId("privacy_service");
    g_isMicEdmMute = PermissionRecordManager::GetInstance().isMicEdmMute_;
    g_isMicMixMute = PermissionRecordManager::GetInstance().isMicMixMute_;
    PermissionRecordManager::GetInstance().isMicEdmMute_ = false;
    PermissionRecordManager::GetInstance().isMicMixMute_ = false;
}

void PermissionRecordManagerTest::TearDownTestCase()
{
    PermissionRecordManager::GetInstance().isMicEdmMute_ = g_isMicEdmMute;
    PermissionRecordManager::GetInstance().isMicMixMute_ = g_isMicMixMute;
}

void PermissionRecordManagerTest::SetUp()
{
    PermissionRecordManager::GetInstance().Init();
    PermissionRecordManager::GetInstance().Register();

    AccessTokenKit::AllocHapToken(g_InfoParms1, g_PolicyPrams1);
    AccessTokenKit::AllocHapToken(g_InfoParms2, g_PolicyPrams2);
    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::PRIVACY, CallerType::MICROPHONE, false);
    if (appStateObserver_ != nullptr) {
        return;
    }
    appStateObserver_ = std::make_shared<PrivacyAppStateObserver>();
}

void PermissionRecordManagerTest::TearDown()
{
    AccessTokenID tokenId = AccessTokenKit::GetHapTokenID(g_InfoParms1.userID, g_InfoParms1.bundleName,
        g_InfoParms1.instIndex);
    AccessTokenKit::DeleteToken(tokenId);
    PrivacyKit::RemovePermissionUsedRecords(tokenId);
    tokenId = AccessTokenKit::GetHapTokenID(g_InfoParms2.userID, g_InfoParms2.bundleName,
        g_InfoParms2.instIndex);
    AccessTokenKit::DeleteToken(tokenId);
    PrivacyKit::RemovePermissionUsedRecords(tokenId);
    appStateObserver_ = nullptr;
    EXPECT_EQ(0, SetSelfTokenID(g_selfTokenId));
}

class PermissionRecordManagerTestCb1 : public StateCustomizedCbk {
public:
    PermissionRecordManagerTestCb1()
    {}

    ~PermissionRecordManagerTestCb1()
    {}

    void StateChangeNotify(AccessTokenID tokenId, bool isShow)
    {
        GTEST_LOG_(INFO) << "PermissionRecordManagerTestCb1 isShow" << isShow;
        isShow_ = isShow;
    }

    void Stop()
    {}

    bool isShow_ = true;
};

/**
 * @tc.name: RegisterPermActiveStatusCallback001
 * @tc.desc: RegisterPermActiveStatusCallback with invalid parameter.
 * @tc.type: FUNC
 * @tc.require: issueI5RWX8
 */
HWTEST_F(PermissionRecordManagerTest, RegisterPermActiveStatusCallback001, TestSize.Level1)
{
    std::vector<std::string> permList = {"ohos.permission.CAMERA"};
    ASSERT_EQ(PrivacyError::ERR_PARAM_INVALID, PermissionRecordManager::GetInstance().RegisterPermActiveStatusCallback(
        GetSelfTokenID(), permList, nullptr));
}


class PermActiveStatusChangeCallback : public PermActiveStatusChangeCallbackStub {
public:
    PermActiveStatusChangeCallback() = default;
    virtual ~PermActiveStatusChangeCallback() = default;

    void ActiveStatusChangeCallback(ActiveChangeResponse& result)
    {
        type_ = result.type;
        GTEST_LOG_(INFO) << "ActiveStatusChange tokenid " << result.tokenID <<
            ", permission " << result.permissionName << ", type " << result.type;
    }

    ActiveChangeType type_ = PERM_INACTIVE;
};

/**
 * @tc.name: RegisterPermActiveStatusCallback002
 * @tc.desc: RegisterPermActiveStatusCallback with exceed limitation.
 * @tc.type: FUNC
 * @tc.require: issueI5RWX8
 */
HWTEST_F(PermissionRecordManagerTest, RegisterPermActiveStatusCallback002, TestSize.Level1)
{
    std::vector<std::string> permList = {"ohos.permission.CAMERA"};
    std::vector<sptr<PermActiveStatusChangeCallback>> callbacks;

    for (size_t i = 0; i < MAX_CALLBACK_SIZE; ++i) {
        sptr<PermActiveStatusChangeCallback> callback = new (std::nothrow) PermActiveStatusChangeCallback();
        ASSERT_NE(nullptr, callback);
        ASSERT_EQ(RET_SUCCESS, PermissionRecordManager::GetInstance().RegisterPermActiveStatusCallback(
            GetSelfTokenID(), permList, callback->AsObject()));
        callbacks.emplace_back(callback);
    }

    sptr<PermActiveStatusChangeCallback> callback = new (std::nothrow) PermActiveStatusChangeCallback();
    ASSERT_NE(nullptr, callback);
    ASSERT_EQ(PrivacyError::ERR_CALLBACKS_EXCEED_LIMITATION,
        PermissionRecordManager::GetInstance().RegisterPermActiveStatusCallback(
            GetSelfTokenID(), permList, callback->AsObject()));

    for (size_t i = 0; i < callbacks.size(); ++i) {
        ASSERT_EQ(RET_SUCCESS,
            PermissionRecordManager::GetInstance().UnRegisterPermActiveStatusCallback(callbacks[i]->AsObject()));
    }
}

/**
 * @tc.name: UnRegisterPermActiveStatusCallback001
 * @tc.desc: UnRegisterPermActiveStatusCallback with invalid parameter.
 * @tc.type: FUNC
 * @tc.require: issueI5RWX8
 */
HWTEST_F(PermissionRecordManagerTest, UnRegisterPermActiveStatusCallback001, TestSize.Level1)
{
    std::vector<std::string> permList = {"ohos.permission.CAMERA"};
    ASSERT_EQ(PrivacyError::ERR_PARAM_INVALID, PermissionRecordManager::GetInstance().RegisterPermActiveStatusCallback(
        GetSelfTokenID(), permList, nullptr));
}

/*
 * @tc.name: StartUsingPermissionTest001
 * @tc.desc: StartUsingPermission function test with invaild tokenId.
 * @tc.type: FUNC
 * @tc.require: issueI5RWX5 issueI5RWX3 issueI5RWXA
 */
HWTEST_F(PermissionRecordManagerTest, StartUsingPermissionTest001, TestSize.Level1)
{
    std::string permissionName = "ohos.permission.CAMERA";
    auto callbackPtr = std::make_shared<PermissionRecordManagerTestCb1>();
    auto callbackWrap = new (std::nothrow) StateChangeCallback(callbackPtr);
    ASSERT_NE(nullptr, callbackPtr);
    ASSERT_NE(nullptr, callbackWrap);
    AccessTokenID tokenId = AccessTokenKit::GetHapTokenID(g_InfoParms1.userID, g_InfoParms1.bundleName,
        g_InfoParms1.instIndex);
    ASSERT_NE(INVALID_TOKENID, tokenId);
    ASSERT_EQ(ERR_PARAM_INVALID, PermissionRecordManager::GetInstance().StartUsingPermission(
        0, PID, permissionName, callbackWrap->AsObject()));
}

/*
 * @tc.name: StartUsingPermissionTest002
 * @tc.desc: StartUsingPermission function test with invaild permissionName.
 * @tc.type: FUNC
 * @tc.require: issueI5RWXF
 */
HWTEST_F(PermissionRecordManagerTest, StartUsingPermissionTest002, TestSize.Level1)
{
    EXPECT_EQ(0, SetSelfTokenID(g_nativeToken));

    auto callbackPtr = std::make_shared<PermissionRecordManagerTestCb1>();
    auto callbackWrap = new (std::nothrow) StateChangeCallback(callbackPtr);
    ASSERT_NE(nullptr, callbackPtr);
    ASSERT_NE(nullptr, callbackWrap);
    AccessTokenID tokenId = AccessTokenKit::GetHapTokenID(g_InfoParms1.userID, g_InfoParms1.bundleName,
        g_InfoParms1.instIndex);
    ASSERT_NE(INVALID_TOKENID, tokenId);
    ASSERT_EQ(ERR_PARAM_INVALID, PermissionRecordManager::GetInstance().StartUsingPermission(
        tokenId, PID, "ohos.permission.LOCATION", callbackWrap->AsObject()));

    // tokenId invaild
    ASSERT_EQ(PrivacyError::ERR_PARAM_INVALID, PermissionRecordManager::GetInstance().StartUsingPermission(
        g_nativeToken, PID, "ohos.permission.CAMERA", nullptr));
    
    ASSERT_EQ(Constant::SUCCESS, PermissionRecordManager::GetInstance().StartUsingPermission(
        tokenId, PID, "ohos.permission.CAMERA", nullptr));
    ASSERT_EQ(PrivacyError::ERR_PERMISSION_ALREADY_START_USING,
        PermissionRecordManager::GetInstance().StartUsingPermission(tokenId, PID, "ohos.permission.CAMERA", nullptr));

    ASSERT_EQ(Constant::SUCCESS,
        PermissionRecordManager::GetInstance().StopUsingPermission(tokenId, PID, "ohos.permission.CAMERA"));
}

/*
 * @tc.name: StartUsingPermissionTest003
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionRecordManagerTest, StartUsingPermissionTest003, TestSize.Level1)
{
    EXPECT_EQ(0, SetSelfTokenID(g_nativeToken));

    char value[VALUE_MAX_LEN] = {0};
    GetParameter(EDM_MIC_MUTE_KEY, "", value, VALUE_MAX_LEN - 1);
    GTEST_LOG_(INFO) << "value:" << value;

    bool isMute = strncmp(value, "true", VALUE_MAX_LEN) == 0;
    SetParameter(EDM_MIC_MUTE_KEY, "true");

    AccessTokenID tokenId = AccessTokenKit::GetHapTokenID(g_InfoParms1.userID, g_InfoParms1.bundleName,
        g_InfoParms1.instIndex);
    ASSERT_NE(static_cast<AccessTokenID>(0), tokenId);
    std::string permissionName = "ohos.permission.MICROPHONE";
    ASSERT_EQ(PrivacyError::ERR_EDM_POLICY_CHECK_FAILED,
        PermissionRecordManager::GetInstance().StartUsingPermission(tokenId, PID, permissionName));
    std::string str = isMute ? "true" : "false";
    SetParameter(EDM_MIC_MUTE_KEY, str.c_str());
}

/*
 * @tc.name: StartUsingPermissionTest004
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionRecordManagerTest, StartUsingPermissionTest004, TestSize.Level1)
{
    EXPECT_EQ(0, SetSelfTokenID(g_nativeToken));

    char value[VALUE_MAX_LEN] = {0};
    GetParameter(EDM_MIC_MUTE_KEY, "", value, VALUE_MAX_LEN - 1);
    GTEST_LOG_(INFO) << "value:" << value;

    bool isMute = strncmp(value, "true", VALUE_MAX_LEN) == 0;
    SetParameter(EDM_MIC_MUTE_KEY, "false");

    ASSERT_EQ(RET_SUCCESS,
        PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::PRIVACY, CallerType::MICROPHONE, true));

    std::vector<std::string> permList = {"ohos.permission.MICROPHONE"};
    sptr<PermActiveStatusChangeCallback> callback = new (std::nothrow) PermActiveStatusChangeCallback();
    ASSERT_NE(nullptr, callback);
    ASSERT_EQ(RET_SUCCESS, PermissionRecordManager::GetInstance().RegisterPermActiveStatusCallback(
        GetSelfTokenID(), permList, callback->AsObject()));

    AccessTokenID tokenId = AccessTokenKit::GetHapTokenID(g_InfoParms1.userID, g_InfoParms1.bundleName,
        g_InfoParms1.instIndex);
    ASSERT_NE(static_cast<AccessTokenID>(0), tokenId);
    std::string permissionName = "ohos.permission.MICROPHONE";
    ASSERT_EQ(RET_SUCCESS,
        PermissionRecordManager::GetInstance().StartUsingPermission(tokenId, PID, permissionName));

    usleep(500000); // 500000us = 0.5s
    ASSERT_EQ(PERM_ACTIVE_IN_BACKGROUND, callback->type_);
    ASSERT_EQ(Constant::SUCCESS,
        PermissionRecordManager::GetInstance().StopUsingPermission(tokenId, PID, permissionName));
    std::string str = isMute ? "true" : "false";
    SetParameter(EDM_MIC_MUTE_KEY, str.c_str());
}

/*
 * @tc.name: StartUsingPermissionTest005
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionRecordManagerTest, StartUsingPermissionTest005, TestSize.Level1)
{
    EXPECT_EQ(0, SetSelfTokenID(g_nativeToken));

    char value[VALUE_MAX_LEN] = {0};
    GetParameter(EDM_MIC_MUTE_KEY, "", value, VALUE_MAX_LEN - 1);
    GTEST_LOG_(INFO) << "value:" << value;

    bool isMute = strncmp(value, "true", VALUE_MAX_LEN) == 0;
    SetParameter(EDM_MIC_MUTE_KEY, "false");

    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::PRIVACY, CallerType::MICROPHONE, false);
    std::vector<std::string> permList = {"ohos.permission.MICROPHONE"};
    sptr<PermActiveStatusChangeCallback> callback = new (std::nothrow) PermActiveStatusChangeCallback();
    ASSERT_NE(nullptr, callback);
    ASSERT_EQ(RET_SUCCESS, PermissionRecordManager::GetInstance().RegisterPermActiveStatusCallback(
        GetSelfTokenID(), permList, callback->AsObject()));

    AccessTokenID tokenId = AccessTokenKit::GetHapTokenID(g_InfoParms1.userID, g_InfoParms1.bundleName,
        g_InfoParms1.instIndex);
    ASSERT_NE(static_cast<AccessTokenID>(0), tokenId);
    std::string permissionName = "ohos.permission.MICROPHONE";
    ASSERT_EQ(RET_SUCCESS,
        PermissionRecordManager::GetInstance().StartUsingPermission(tokenId, PID, permissionName));

    usleep(500000); // 500000us = 0.5s
    ASSERT_EQ(PERM_ACTIVE_IN_BACKGROUND, callback->type_);
    ASSERT_EQ(Constant::SUCCESS,
        PermissionRecordManager::GetInstance().StopUsingPermission(tokenId, PID, permissionName));

    std::string str = isMute ? "true" : "false";
    SetParameter(EDM_MIC_MUTE_KEY, str.c_str());
}

/*
 * @tc.name: StartUsingPermissionTest006
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionRecordManagerTest, StartUsingPermissionTest006, TestSize.Level1)
{
    EXPECT_EQ(0, SetSelfTokenID(g_nativeToken));

    char value[VALUE_MAX_LEN] = {0};
    GetParameter(EDM_MIC_MUTE_KEY, "", value, VALUE_MAX_LEN - 1);
    GTEST_LOG_(INFO) << "value:" << value;

    bool isMute = strncmp(value, "true", VALUE_MAX_LEN) == 0;
    SetParameter(EDM_MIC_MUTE_KEY, "true");

    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::PRIVACY, CallerType::MICROPHONE, true);
    std::vector<std::string> permList = {"ohos.permission.LOCATION"};
    sptr<PermActiveStatusChangeCallback> callback = new (std::nothrow) PermActiveStatusChangeCallback();
    ASSERT_NE(nullptr, callback);
    ASSERT_EQ(RET_SUCCESS, PermissionRecordManager::GetInstance().RegisterPermActiveStatusCallback(
        GetSelfTokenID(), permList, callback->AsObject()));

    AccessTokenID tokenId = AccessTokenKit::GetHapTokenID(g_InfoParms1.userID, g_InfoParms1.bundleName,
        g_InfoParms1.instIndex);
    ASSERT_NE(static_cast<AccessTokenID>(0), tokenId);
    std::string permissionName = "ohos.permission.LOCATION";
    ASSERT_EQ(RET_SUCCESS,
        PermissionRecordManager::GetInstance().StartUsingPermission(tokenId, PID, permissionName));

    usleep(500000); // 500000us = 0.5s
    ASSERT_EQ(PERM_ACTIVE_IN_BACKGROUND, callback->type_);
    ASSERT_EQ(Constant::SUCCESS,
        PermissionRecordManager::GetInstance().StopUsingPermission(tokenId, PID, permissionName));

    std::string str = isMute ? "true" : "false";
    SetParameter(EDM_MIC_MUTE_KEY, str.c_str());
}

/*
 * @tc.name: StartUsingPermissionTest007
 * @tc.desc: PermissionRecordManager::StartUsingPermission function test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionRecordManagerTest, StartUsingPermissionTest007, TestSize.Level1)
{
    AccessTokenID tokenId = AccessTokenKit::GetHapTokenID(g_InfoParms1.userID, g_InfoParms1.bundleName,
        g_InfoParms1.instIndex);
    ASSERT_NE(static_cast<AccessTokenID>(0), tokenId);

    // tokenId invaild
    ASSERT_EQ(PrivacyError::ERR_PARAM_INVALID, PermissionRecordManager::GetInstance().StartUsingPermission(
        g_nativeToken, PID, "ohos.permission.READ_MEDIA"));

    ASSERT_EQ(Constant::SUCCESS, PermissionRecordManager::GetInstance().StartUsingPermission(
        tokenId, PID, "ohos.permission.READ_MEDIA"));
    ASSERT_EQ(PrivacyError::ERR_PERMISSION_ALREADY_START_USING,
        PermissionRecordManager::GetInstance().StartUsingPermission(tokenId, PID, "ohos.permission.READ_MEDIA"));

    ASSERT_EQ(Constant::SUCCESS,
        PermissionRecordManager::GetInstance().StopUsingPermission(tokenId, PID, "ohos.permission.READ_MEDIA"));
}

/*
 * @tc.name: StartUsingPermissionTest008
 * @tc.desc: Test multiple process start using permission
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionRecordManagerTest, StartUsingPermissionTest008, TestSize.Level1)
{
    EXPECT_EQ(0, SetSelfTokenID(g_nativeToken));

    std::vector<std::string> permList = {"ohos.permission.CAMERA"};
    sptr<PermActiveStatusChangeCallback> callback = new (std::nothrow) PermActiveStatusChangeCallback();
    ASSERT_NE(nullptr, callback);
    ASSERT_EQ(RET_SUCCESS, PermissionRecordManager::GetInstance().RegisterPermActiveStatusCallback(
        GetSelfTokenID(), permList, callback->AsObject()));

    AccessTokenID tokenId = AccessTokenKit::GetHapTokenID(g_InfoParms1.userID, g_InfoParms1.bundleName,
        g_InfoParms1.instIndex);
    ASSERT_NE(static_cast<AccessTokenID>(0), tokenId);
    std::string permissionName = "ohos.permission.CAMERA";
    ASSERT_EQ(RET_SUCCESS,
        PermissionRecordManager::GetInstance().StartUsingPermission(tokenId, TEST_PID_1, permissionName));
    ASSERT_EQ(RET_SUCCESS,
        PermissionRecordManager::GetInstance().StartUsingPermission(tokenId, TEST_PID_3, permissionName));
    ProcessData processData;
    processData.accessTokenId = tokenId;
    processData.pid = TEST_PID_1;
    appStateObserver_->OnProcessDied(processData);
    usleep(500000); // 500000us = 0.5s
    ASSERT_EQ(PERM_ACTIVE_IN_BACKGROUND, callback->type_);

    processData.pid = TEST_PID_2;
    appStateObserver_->OnProcessDied(processData);
    usleep(500000); // 500000us = 0.5s
    ASSERT_EQ(PERM_ACTIVE_IN_BACKGROUND, callback->type_);

    processData.pid = TEST_PID_3;
    appStateObserver_->OnProcessDied(processData);
    usleep(500000); // 500000us = 0.5s
    ASSERT_EQ(PERM_INACTIVE, callback->type_);
}

/*
 * @tc.name: StartUsingPermissionTest009
 * @tc.desc: Test multiple process start using permission
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionRecordManagerTest, StartUsingPermissionTest009, TestSize.Level1)
{
    EXPECT_EQ(0, SetSelfTokenID(g_nativeToken));

    auto callbackPtr1 = std::make_shared<PermissionRecordManagerTestCb1>();
    auto callbackWrap1 = new (std::nothrow) StateChangeCallback(callbackPtr1);
    ASSERT_NE(nullptr, callbackPtr1);
    ASSERT_NE(nullptr, callbackWrap1);

    auto callbackPtr2 = std::make_shared<PermissionRecordManagerTestCb1>();
    auto callbackWrap2 = new (std::nothrow) StateChangeCallback(callbackPtr2);
    ASSERT_NE(nullptr, callbackPtr2);
    ASSERT_NE(nullptr, callbackWrap2);

    AccessTokenID tokenId = AccessTokenKit::GetHapTokenID(g_InfoParms1.userID, g_InfoParms1.bundleName,
        g_InfoParms1.instIndex);
    ASSERT_NE(static_cast<AccessTokenID>(0), tokenId);
    std::string permissionName = "ohos.permission.CAMERA";

    ASSERT_EQ(RET_SUCCESS, PermissionRecordManager::GetInstance().StartUsingPermission(
        tokenId, TEST_PID_1, permissionName, callbackWrap1->AsObject()));
    ASSERT_EQ(RET_SUCCESS, PermissionRecordManager::GetInstance().StartUsingPermission(
        tokenId, TEST_PID_2, permissionName, callbackWrap2->AsObject()));

    AppStateData appStateData;
    appStateData.accessTokenId = tokenId;
    appStateData.state = static_cast<int32_t>(ApplicationState::APP_STATE_FOREGROUND);
    appStateData.pid = TEST_PID_1;
    appStateObserver_->OnAppStateChanged(appStateData);
    appStateData.pid = TEST_PID_2;
    appStateObserver_->OnAppStateChanged(appStateData);

    appStateData.state = static_cast<int32_t>(ApplicationState::APP_STATE_BACKGROUND);
    appStateData.pid = TEST_PID_1;
    appStateObserver_->OnAppStateChanged(appStateData);
    usleep(500000); // 500000us = 0.5s
    ASSERT_FALSE(callbackPtr1->isShow_);
    ASSERT_TRUE(callbackPtr2->isShow_);

    appStateData.pid = TEST_PID_2;
    appStateObserver_->OnAppStateChanged(appStateData);
    usleep(500000); // 500000us = 0.5s
    ASSERT_FALSE(callbackPtr2->isShow_);
}

/*
 * @tc.name: StartUsingPermissionTest010
 * @tc.desc: Test multiple process start using permission
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionRecordManagerTest, StartUsingPermissionTest010, TestSize.Level1)
{
    EXPECT_EQ(0, SetSelfTokenID(g_nativeToken));

    std::vector<std::string> permList = {"ohos.permission.CAMERA"};
    sptr<PermActiveStatusChangeCallback> callback = new (std::nothrow) PermActiveStatusChangeCallback();
    ASSERT_NE(nullptr, callback);
    ASSERT_EQ(RET_SUCCESS, PermissionRecordManager::GetInstance().RegisterPermActiveStatusCallback(
        GetSelfTokenID(), permList, callback->AsObject()));

    AccessTokenID tokenId = AccessTokenKit::GetHapTokenID(g_InfoParms1.userID, g_InfoParms1.bundleName,
        g_InfoParms1.instIndex);
    ASSERT_NE(static_cast<AccessTokenID>(0), tokenId);
    std::string permissionName = "ohos.permission.CAMERA";
    ASSERT_EQ(RET_SUCCESS,
        PermissionRecordManager::GetInstance().StartUsingPermission(tokenId, TEST_PID_1, permissionName));
    ASSERT_EQ(RET_SUCCESS,
        PermissionRecordManager::GetInstance().StartUsingPermission(tokenId, TEST_PID_2, permissionName));

    ASSERT_EQ(Constant::SUCCESS,
        PermissionRecordManager::GetInstance().StopUsingPermission(tokenId, TEST_PID_2, "ohos.permission.CAMERA"));
    usleep(500000); // 500000us = 0.5s
    ASSERT_EQ(PERM_ACTIVE_IN_BACKGROUND, callback->type_);

    ASSERT_EQ(Constant::SUCCESS,
        PermissionRecordManager::GetInstance().StopUsingPermission(tokenId, TEST_PID_1, "ohos.permission.CAMERA"));
    usleep(500000); // 500000us = 0.5s
    ASSERT_EQ(PERM_INACTIVE, callback->type_);
}

/*
 * @tc.name: StartUsingPermissionTest011
 * @tc.desc: Test default pid -1 start using permission and OnProcessDied
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionRecordManagerTest, StartUsingPermissionTest011, TestSize.Level1)
{
    EXPECT_EQ(0, SetSelfTokenID(g_nativeToken));

    std::vector<std::string> permList = {"ohos.permission.CAMERA"};
    sptr<PermActiveStatusChangeCallback> callback = new (std::nothrow) PermActiveStatusChangeCallback();
    ASSERT_NE(nullptr, callback);
    ASSERT_EQ(RET_SUCCESS, PermissionRecordManager::GetInstance().RegisterPermActiveStatusCallback(
        GetSelfTokenID(), permList, callback->AsObject()));
    AccessTokenID tokenId = AccessTokenKit::GetHapTokenID(g_InfoParms1.userID, g_InfoParms1.bundleName,
        g_InfoParms1.instIndex);
    ASSERT_NE(static_cast<AccessTokenID>(0), tokenId);
    std::string permissionName = "ohos.permission.CAMERA";
    ASSERT_EQ(RET_SUCCESS,
        PermissionRecordManager::GetInstance().StartUsingPermission(tokenId, PID, permissionName));
    ASSERT_EQ(PrivacyError::ERR_PERMISSION_ALREADY_START_USING,
        PermissionRecordManager::GetInstance().StartUsingPermission(tokenId, PID, permissionName));

    // makesure callback end
    usleep(500000); // 500000us = 0.5s
    callback->type_ = PERM_TEMPORARY_CALL;
    ProcessData processData;
    processData.accessTokenId = tokenId;
    processData.pid = PID;
    appStateObserver_->OnProcessDied(processData);
    usleep(500000);
    ASSERT_EQ(PERM_TEMPORARY_CALL, callback->type_);
}

#ifndef APP_SECURITY_PRIVACY_SERVICE
/*
 * @tc.name: ShowGlobalDialog001
 * @tc.desc: ShowGlobalDialog function test
 * @tc.type: FUNC
 * @tc.require: issueI5RWXF
 */
HWTEST_F(PermissionRecordManagerTest, ShowGlobalDialog001, TestSize.Level1)
{
    EXPECT_EQ(0, SetSelfTokenID(g_nativeToken));

    ASSERT_EQ(true, PermissionRecordManager::GetInstance().ShowGlobalDialog("ohos.permission.CAMERA"));
    sleep(3); // wait for dialog disappear
    ASSERT_EQ(true, PermissionRecordManager::GetInstance().ShowGlobalDialog("ohos.permission.MICROPHONE"));
    sleep(3); // wait for dialog disappear
    ASSERT_EQ(true, PermissionRecordManager::GetInstance().ShowGlobalDialog("ohos.permission.LOCATION")); // no dialog
}
#endif

/*
 * @tc.name: AppStateChangeListener001
 * @tc.desc: AppStateChange function test mic global switch is close
 * @tc.type: FUNC
 * @tc.require: issueI5RWXF
 */
HWTEST_F(PermissionRecordManagerTest, AppStateChangeListener001, TestSize.Level1)
{
    EXPECT_EQ(0, SetSelfTokenID(g_nativeToken));
    
    char value[VALUE_MAX_LEN] = {0};
    GetParameter(EDM_MIC_MUTE_KEY, "", value, VALUE_MAX_LEN - 1);
    GTEST_LOG_(INFO) << "value:" << value;

    bool isMute = strncmp(value, "true", VALUE_MAX_LEN) == 0;
    SetParameter(EDM_MIC_MUTE_KEY, std::to_string(false).c_str());

    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::PRIVACY, CallerType::MICROPHONE, true);
    AccessTokenID tokenId = AccessTokenKit::GetHapTokenID(g_InfoParms1.userID, g_InfoParms1.bundleName,
        g_InfoParms1.instIndex);
    // status is inactive
    ASSERT_EQ(RET_SUCCESS,
        PermissionRecordManager::GetInstance().StartUsingPermission(tokenId, PID, "ohos.permission.MICROPHONE"));
    sleep(3); // wait for dialog disappear
    PermissionRecordManager::GetInstance().NotifyAppStateChange(tokenId, PID, PERM_ACTIVE_IN_BACKGROUND);
    ASSERT_EQ(RET_SUCCESS,
        PermissionRecordManager::GetInstance().StopUsingPermission(tokenId, PID, "ohos.permission.MICROPHONE"));
    std::string str = isMute ? "true" : "false";
    SetParameter(EDM_MIC_MUTE_KEY, str.c_str());
}

/*
 * @tc.name: TransferOpcodeToPermission001
 * @tc.desc: Constant::TransferOpcodeToPermission function test return false
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionRecordManagerTest, TransferOpcodeToPermission001, TestSize.Level1)
{
    int32_t opCode = static_cast<int32_t>(Constant::OpCode::OP_INVALID);
    std::string permissionName;
    ASSERT_EQ(false, Constant::TransferOpcodeToPermission(opCode, permissionName));
}

/*
 * @tc.name: AddPermissionUsedRecord001
 * @tc.desc: PermissionRecordManager::AddPermissionUsedRecord function test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionRecordManagerTest, AddPermissionUsedRecord001, TestSize.Level1)
{
    AccessTokenID tokenId = AccessTokenKit::GetHapTokenID(g_InfoParms1.userID, g_InfoParms1.bundleName,
        g_InfoParms1.instIndex);
    ASSERT_NE(static_cast<AccessTokenID>(0), tokenId);

    AddPermParamInfo info;
    info.tokenId = tokenId;
    info.permissionName = "com.ohos.test";
    info.successCount = 1;
    info.failCount = 0;

    ASSERT_EQ(PrivacyError::ERR_PERMISSION_NOT_EXIST, PermissionRecordManager::GetInstance().AddPermissionUsedRecord(
        info)); // invaild permission error

    info.type = PermissionUsedType::PICKER_TYPE;
    ASSERT_EQ(PrivacyError::ERR_PERMISSION_NOT_EXIST, PermissionRecordManager::GetInstance().AddPermissionUsedRecord(
        info)); // invaild permission error

    info.type = PermissionUsedType::SECURITY_COMPONENT_TYPE;
    ASSERT_EQ(PrivacyError::ERR_PERMISSION_NOT_EXIST, PermissionRecordManager::GetInstance().AddPermissionUsedRecord(
        info)); // invaild permission error
}

/*
 * @tc.name: AddPermissionUsedRecord002
 * @tc.desc: PermissionRecordManager::AddPermissionUsedRecord function test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionRecordManagerTest, AddPermissionUsedRecord002, TestSize.Level1)
{
    AccessTokenID tokenId = AccessTokenKit::GetHapTokenID(g_InfoParms1.userID, g_InfoParms1.bundleName,
        g_InfoParms1.instIndex);
    ASSERT_NE(static_cast<AccessTokenID>(0), tokenId);

    AddPermParamInfo info;
    info.tokenId = tokenId;
    info.permissionName = "com.permission.READ_MEDIA";
    info.successCount = 0;
    info.failCount = 0;
    ASSERT_EQ(PrivacyError::ERR_PARAM_INVALID, PermissionRecordManager::GetInstance().AddPermissionUsedRecord(info));
}

/*
 * @tc.name: RemovePermissionUsedRecords001
 * @tc.desc: PermissionRecordManager::RemovePermissionUsedRecords function test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionRecordManagerTest, RemovePermissionUsedRecords001, TestSize.Level1)
{
    AccessTokenID tokenId = AccessTokenKit::GetHapTokenID(g_InfoParms1.userID, g_InfoParms1.bundleName,
        g_InfoParms1.instIndex);
    ASSERT_NE(static_cast<AccessTokenID>(0), tokenId);
}

/*
 * @tc.name: StopUsingPermission001
 * @tc.desc: PermissionRecordManager::StopUsingPermission function test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionRecordManagerTest, StopUsingPermission001, TestSize.Level1)
{
    AccessTokenID tokenId = AccessTokenKit::GetHapTokenID(g_InfoParms1.userID, g_InfoParms1.bundleName,
        g_InfoParms1.instIndex);
    ASSERT_NE(static_cast<AccessTokenID>(0), tokenId);

    // tokenId invaild
    ASSERT_EQ(PrivacyError::ERR_PARAM_INVALID, PermissionRecordManager::GetInstance().StopUsingPermission(
        static_cast<AccessTokenID>(0), PID, "ohos.permission.READ_MEDIA"));

    // permission invaild
    ASSERT_EQ(PrivacyError::ERR_PERMISSION_NOT_EXIST, PermissionRecordManager::GetInstance().StopUsingPermission(
        tokenId, PID, "ohos.permission.test"));

    // not start using
    ASSERT_EQ(PrivacyError::ERR_PERMISSION_NOT_START_USING,
        PermissionRecordManager::GetInstance().StopUsingPermission(tokenId, PID, "ohos.permission.READ_MEDIA"));
}

/*
 * @tc.name: RegisterPermActiveStatusCallback003
 * @tc.desc: PermissionRecordManager::RegisterPermActiveStatusCallback function test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionRecordManagerTest, RegisterPermActiveStatusCallback003, TestSize.Level1)
{
    std::vector<std::string> permList;

    permList.emplace_back("com.ohos.TEST");
    // GetDefPermission != Constant::SUCCESS && listRes is empty && listSrc is not empty
    ASSERT_EQ(PrivacyError::ERR_PARAM_INVALID, PermissionRecordManager::GetInstance().RegisterPermActiveStatusCallback(
        GetSelfTokenID(), permList, nullptr));
}

/*
 * @tc.name: GetPermissionUsedType001
 * @tc.desc: PermissionRecordManager::GetPermissionUsedType function test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionRecordManagerTest, GetPermissionUsedType001, TestSize.Level1)
{
    int32_t tokenId = RANDOM_TOKENID;
    std::string permissionName = "ohos.permission.PERMISSION_RECORD_MANAGER_TEST";
    std::vector<PermissionUsedTypeInfo> results;
    // tokenId is not exsit
    ASSERT_EQ(PrivacyError::ERR_TOKENID_NOT_EXIST,
        PermissionRecordManager::GetInstance().GetPermissionUsedTypeInfos(tokenId, permissionName, results));

    tokenId = 0;
    // permissionName is not exsit
    ASSERT_EQ(PrivacyError::ERR_PERMISSION_NOT_EXIST,
        PermissionRecordManager::GetInstance().GetPermissionUsedTypeInfos(tokenId, permissionName, results));

    permissionName = "ohos.permission.CAMERA";
    ASSERT_EQ(0, PermissionRecordManager::GetInstance().GetPermissionUsedTypeInfos(tokenId, permissionName, results));
}

/**
 * @tc.name: Dlopen001
 * @tc.desc: Open a not exist lib & not exist func
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionRecordManagerTest, Dlopen001, TestSize.Level1)
{
    LibraryLoader loader1("libnotexist.z.so"); // is a not exist path
    EXPECT_EQ(nullptr, loader1.handle_);

    LibraryLoader loader2("libaccesstoken_manager_service.z.so"); // is a exist lib without create func
    EXPECT_EQ(nullptr, loader2.instance_);
    EXPECT_NE(nullptr, loader2.handle_);
}

/*
 * @tc.name: AddDataValueToResults001
 * @tc.desc: PermissionRecordManager::AddDataValueToResults function test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionRecordManagerTest, AddDataValueToResults001, TestSize.Level1)
{
    GenericValues value;
    value.Put(PrivacyFiledConst::FIELD_TOKEN_ID, RANDOM_TOKENID);
    value.Put(PrivacyFiledConst::FIELD_USED_TYPE, NORMAL_TYPE_ADD_VALUE);
    std::vector<PermissionUsedTypeInfo> results;

    PermissionRecordManager::GetInstance().AddDataValueToResults(value, results);
    ASSERT_EQ(PermissionUsedType::NORMAL_TYPE, results[FIRST_INDEX].type);
}

/*
 * @tc.name: AddDataValueToResults002
 * @tc.desc: PermissionRecordManager::AddDataValueToResults function test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionRecordManagerTest, AddDataValueToResults002, TestSize.Level1)
{
    GenericValues value;
    value.Put(PrivacyFiledConst::FIELD_TOKEN_ID, RANDOM_TOKENID);
    value.Put(PrivacyFiledConst::FIELD_USED_TYPE, PICKER_TYPE_ADD_VALUE);
    std::vector<PermissionUsedTypeInfo> results;

    PermissionRecordManager::GetInstance().AddDataValueToResults(value, results);
    ASSERT_EQ(PermissionUsedType::PICKER_TYPE, results[FIRST_INDEX].type);
}

/*
 * @tc.name: AddDataValueToResults003
 * @tc.desc: PermissionRecordManager::AddDataValueToResults function test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionRecordManagerTest, AddDataValueToResults003, TestSize.Level1)
{
    GenericValues value;
    value.Put(PrivacyFiledConst::FIELD_TOKEN_ID, RANDOM_TOKENID);
    value.Put(PrivacyFiledConst::FIELD_USED_TYPE, SEC_COMPONENT_TYPE_ADD_VALUE);
    std::vector<PermissionUsedTypeInfo> results;

    PermissionRecordManager::GetInstance().AddDataValueToResults(value, results);
    ASSERT_EQ(PermissionUsedType::SECURITY_COMPONENT_TYPE, results[FIRST_INDEX].type);
    ASSERT_EQ(0, GetFirstCallerTokenID());
}

/*
 * @tc.name: SetMutePolicyTest001
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionRecordManagerTest, SetMutePolicyTest001, TestSize.Level1)
{
    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::EDM, CallerType::MICROPHONE, true);
    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::PRIVACY, CallerType::MICROPHONE, true);
    EXPECT_EQ(RET_SUCCESS,
        PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::EDM, CallerType::MICROPHONE, true));

    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::EDM, CallerType::MICROPHONE, true);
    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::PRIVACY, CallerType::MICROPHONE, true);
    EXPECT_EQ(ERR_PRIVACY_POLICY_CHECK_FAILED,
        PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::EDM, CallerType::MICROPHONE, false));
}

/*
 * @tc.name: SetMutePolicyTest002
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionRecordManagerTest, SetMutePolicyTest002, TestSize.Level1)
{
    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::EDM, CallerType::MICROPHONE, false);
    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::PRIVACY, CallerType::MICROPHONE, true);
    EXPECT_EQ(RET_SUCCESS,
        PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::EDM, CallerType::MICROPHONE, true));

    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::EDM, CallerType::MICROPHONE, false);
    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::PRIVACY, CallerType::MICROPHONE, true);
    EXPECT_EQ(ERR_PRIVACY_POLICY_CHECK_FAILED,
        PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::EDM, CallerType::MICROPHONE, false));
}

/*
 * @tc.name: SetMutePolicyTest003
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionRecordManagerTest, SetMutePolicyTest003, TestSize.Level1)
{
    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::EDM, CallerType::MICROPHONE, false);
    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::PRIVACY, CallerType::MICROPHONE, false);
    EXPECT_EQ(RET_SUCCESS,
        PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::EDM, CallerType::MICROPHONE, true));

    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::EDM, CallerType::MICROPHONE, false);
    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::PRIVACY, CallerType::MICROPHONE, false);
    EXPECT_EQ(RET_SUCCESS,
        PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::EDM, CallerType::MICROPHONE, false));
}

/*
 * @tc.name: SetMutePolicyTest004
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionRecordManagerTest, SetMutePolicyTest004, TestSize.Level1)
{
    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::EDM, CallerType::MICROPHONE, true);
    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::PRIVACY, CallerType::MICROPHONE, true);
    EXPECT_EQ(RET_SUCCESS,
        PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::PRIVACY, CallerType::MICROPHONE, true));

    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::EDM, CallerType::MICROPHONE, true);
    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::PRIVACY, CallerType::MICROPHONE, true);
    EXPECT_EQ(ERR_EDM_POLICY_CHECK_FAILED,
        PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::PRIVACY, CallerType::MICROPHONE, false));
}

/*
 * @tc.name: SetMutePolicyTest005
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionRecordManagerTest, SetMutePolicyTest005, TestSize.Level1)
{
    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::EDM, CallerType::MICROPHONE, false);
    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::PRIVACY, CallerType::MICROPHONE, true);
    EXPECT_EQ(RET_SUCCESS,
        PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::PRIVACY, CallerType::MICROPHONE, true));

    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::EDM, CallerType::MICROPHONE, false);
    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::PRIVACY, CallerType::MICROPHONE, true);
    EXPECT_EQ(RET_SUCCESS,
        PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::PRIVACY, CallerType::MICROPHONE, false));
}

/*
 * @tc.name: SetMutePolicyTest006
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionRecordManagerTest, SetMutePolicyTest006, TestSize.Level1)
{
    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::EDM, CallerType::MICROPHONE, false);
    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::PRIVACY, CallerType::MICROPHONE, false);
    EXPECT_EQ(RET_SUCCESS,
        PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::PRIVACY, CallerType::MICROPHONE, true));

    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::EDM, CallerType::MICROPHONE, false);
    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::PRIVACY, CallerType::MICROPHONE, false);
    EXPECT_EQ(RET_SUCCESS,
        PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::PRIVACY, CallerType::MICROPHONE, false));
}

/*
 * @tc.name: SetMutePolicyTest007
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionRecordManagerTest, SetMutePolicyTest007, TestSize.Level1)
{
    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::EDM, CallerType::MICROPHONE, true);
    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::PRIVACY, CallerType::MICROPHONE, true);
    EXPECT_EQ(RET_SUCCESS,
        PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::TEMPORARY, CallerType::MICROPHONE, true));

    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::EDM, CallerType::MICROPHONE, true);
    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::PRIVACY, CallerType::MICROPHONE, true);
    EXPECT_EQ(ERR_EDM_POLICY_CHECK_FAILED,
        PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::TEMPORARY, CallerType::MICROPHONE, false));
}

#ifndef APP_SECURITY_PRIVACY_SERVICE
/*
 * @tc.name: SetMutePolicyTest008
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionRecordManagerTest, SetMutePolicyTest008, TestSize.Level1)
{
    EXPECT_EQ(0, SetSelfTokenID(g_nativeToken));
    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::EDM, CallerType::MICROPHONE, false);
    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::PRIVACY, CallerType::MICROPHONE, true);
    EXPECT_EQ(RET_SUCCESS,
        PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::TEMPORARY, CallerType::MICROPHONE, true));

    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::EDM, CallerType::MICROPHONE, false);
    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::PRIVACY, CallerType::MICROPHONE, true);
    EXPECT_EQ(ERR_PRIVACY_POLICY_CHECK_FAILED,
        PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::TEMPORARY, CallerType::MICROPHONE, false));
}
#endif

/*
 * @tc.name: SetMutePolicyTest009
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionRecordManagerTest, SetMutePolicyTest009, TestSize.Level1)
{
    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::EDM, CallerType::MICROPHONE, false);
    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::PRIVACY, CallerType::MICROPHONE, false);
    EXPECT_EQ(RET_SUCCESS,
        PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::TEMPORARY, CallerType::MICROPHONE, true));

    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::EDM, CallerType::MICROPHONE, false);
    PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::PRIVACY, CallerType::MICROPHONE, false);
    EXPECT_EQ(RET_SUCCESS,
        PermissionRecordManager::GetInstance().SetMutePolicy(PolicyType::TEMPORARY, CallerType::MICROPHONE, false));
}
} // namespace AccessToken
} // namespace Security
} // namespace OHOS
