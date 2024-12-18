/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "native_token_receptor_test.h"

#include <fcntl.h>
#include <memory>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "accesstoken_info_manager.h"
#include "accesstoken_kit.h"
#include "access_token_db.h"
#include "access_token_error.h"
#include "permission_definition_cache.h"
#include "permission_manager.h"
#include "permission_state_full.h"
#include "token_field_const.h"
#define private public
#include "nativetoken_kit.h"
#include "native_token_receptor.h"
#undef private
#include "securec.h"

using namespace testing::ext;
using namespace OHOS::Security::AccessToken;

namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_ACCESSTOKEN, "NativeTokenReceptorTest"};
}

void NativeTokenReceptorTest::SetUpTestCase()
{
    // delete all test 0x28100000 - 0x28100007
    for (unsigned int i = 0x28100000; i <= 0x28100007; i++) {
        AccessTokenInfoManager::GetInstance().RemoveNativeTokenInfo(i);
    }
    PermissionDef infoManagerPermDef = {
        .permissionName = "ohos.permission.DISCOVER_BLUETOOTH",
        .bundleName = "accesstoken_test",
        .grantMode = USER_GRANT,
        .availableLevel = APL_NORMAL,
        .provisionEnable = false,
        .distributedSceneEnable = false,
        .label = "label",
        .labelId = 1,
        .description = "CAMERA",
        .descriptionId = 1
    };
    PermissionDefinitionCache::GetInstance().Insert(infoManagerPermDef, 1);
    infoManagerPermDef.permissionName = "ohos.permission.MANAGE_USER_IDM";
    PermissionDefinitionCache::GetInstance().Insert(infoManagerPermDef, 1);
    infoManagerPermDef.permissionName = "ohos.permission.ACCELEROMETER";
    PermissionDefinitionCache::GetInstance().Insert(infoManagerPermDef, 1);
}

void NativeTokenReceptorTest::TearDownTestCase()
{
    sleep(3); // delay 3 minutes
}

void NativeTokenReceptorTest::SetUp()
{}

void NativeTokenReceptorTest::TearDown()
{
    ACCESSTOKEN_LOG_INFO(LABEL, "test down!");
}

/**
 * @tc.name: ParserNativeRawData001
 * @tc.desc: Verify processing right native token json.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(NativeTokenReceptorTest, ParserNativeRawData001, TestSize.Level1)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "test ParserNativeRawData001!");
    std::string testStr = R"([)"\
        R"({"processName":"process6","APL":3,"version":1,"tokenId":685266937,"tokenAttr":0,)"\
        R"("dcaps":["AT_CAP","ST_CAP"], "permissions":[], "nativeAcls":[]},)"\
        R"({"processName":"process5","APL":3,"version":1,"tokenId":678065606,"tokenAttr":0,)"\
        R"("dcaps":["AT_CAP","ST_CAP"], "permissions":[], "nativeAcls":[]}])";

    NativeTokenReceptor& receptor = NativeTokenReceptor::GetInstance();
    std::vector<std::shared_ptr<NativeTokenInfoInner>> tokenInfos;
    receptor.ParserNativeRawData(testStr, tokenInfos);
    ASSERT_EQ(static_cast<uint32_t>(2), tokenInfos.size());
    ASSERT_NE(nullptr, tokenInfos[0]);
    ASSERT_NE(nullptr, tokenInfos[1]);

    ASSERT_EQ("process6", tokenInfos[0]->GetProcessName());
    ASSERT_EQ(static_cast<AccessTokenID>(685266937), tokenInfos[0]->GetTokenID());

    ASSERT_EQ("process5", tokenInfos[1]->GetProcessName());
    ASSERT_EQ(static_cast<AccessTokenID>(678065606), tokenInfos[1]->GetTokenID());
}

/**
 * @tc.name: ParserNativeRawData002
 * @tc.desc: Verify processing wrong native token json.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(NativeTokenReceptorTest, ParserNativeRawData002, TestSize.Level1)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "test ParserNativeRawData002!");
    std::string testStr = R"([{"processName":""}])";
    std::vector<std::shared_ptr<NativeTokenInfoInner>> tokenInfos;

    NativeTokenReceptor& receptor = NativeTokenReceptor::GetInstance();

    receptor.ParserNativeRawData(testStr, tokenInfos);
    ASSERT_EQ(static_cast<uint32_t>(0), tokenInfos.size());

    std::string testStr1 = R"([{"processName":"", }])";
    receptor.ParserNativeRawData(testStr1, tokenInfos);
    ASSERT_EQ(static_cast<uint32_t>(0), tokenInfos.size());

    std::string testStr2 = R"([{"processName":"process6"}, {}])";
    receptor.ParserNativeRawData(testStr2, tokenInfos);
    ASSERT_EQ(static_cast<uint32_t>(0), tokenInfos.size());

    std::string testStr3 = R"([{"processName":""}, {"":"", ""}])";
    receptor.ParserNativeRawData(testStr3, tokenInfos);
    ASSERT_EQ(static_cast<uint32_t>(0), tokenInfos.size());

    std::string testStr4 = R"([{"processName":"process6", "tokenId":685266937, "APL":3, "version":new}])";
    receptor.ParserNativeRawData(testStr4, tokenInfos);
    ASSERT_EQ(static_cast<uint32_t>(0), tokenInfos.size());

    std::string testStr5 = R"([{"processName":"process6", "tokenId":685266937, "APL":7, "version":1}])";
    receptor.ParserNativeRawData(testStr5, tokenInfos);
    ASSERT_EQ(static_cast<uint32_t>(0), tokenInfos.size());

    std::string testStr6 =
        R"({"NativeToken":[{"processName":"process6", "tokenId":685266937, "APL":7, "version":1}]})";
    receptor.ParserNativeRawData(testStr6, tokenInfos);
    ASSERT_EQ(static_cast<uint32_t>(0), tokenInfos.size());

    std::string testStr7 = R"({"NativeToken":[{"processName":"process6", "tokenId":685266937, "APL":7, "version":1}])";
    receptor.ParserNativeRawData(testStr7, tokenInfos);
    ASSERT_EQ(static_cast<uint32_t>(0), tokenInfos.size());

    std::string testStr8 = R"(["NativeToken":])";
    receptor.ParserNativeRawData(testStr8, tokenInfos);
    ASSERT_EQ(static_cast<uint32_t>(0), tokenInfos.size());

    std::string testStr9 = R"([)";
    receptor.ParserNativeRawData(testStr9, tokenInfos);
    ASSERT_EQ(static_cast<uint32_t>(0), tokenInfos.size());
}

namespace OHOS {
namespace Security {
namespace AccessToken {
    extern void from_json(const nlohmann::json& j, std::shared_ptr<NativeTokenInfoInner>& p);
}
}
}

/**
 * @tc.name: from_json001
 * @tc.desc: Verify from json right case.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(NativeTokenReceptorTest, from_json001, TestSize.Level1)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "test from_json001!");
    nlohmann::json j = nlohmann::json{
        {"processName", "process6"},
        {"APL", APL_SYSTEM_CORE},
        {"version", 1},
        {"tokenId", 685266937},
        {"tokenAttr", 0},
        {"dcaps", {"AT_CAP", "ST_CAP"}},
        {"permissions", {"ohos.permission.PLACE_CALL"}},
        {"nativeAcls", {"ohos.permission.PLACE_CALL"}}};
    std::shared_ptr<NativeTokenInfoInner> p;
    from_json(j, p);
    ASSERT_NE((p == nullptr), true);
}

/**
 * @tc.name: from_json002
 * @tc.desc: Verify from json wrong case.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(NativeTokenReceptorTest, from_json002, TestSize.Level1)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "test from_json002!");
    // version wrong
    nlohmann::json j = nlohmann::json{
        {"processName", "process6"}, {"APL", APL_SYSTEM_CORE},
        {"version", 2}, {"tokenId", 685266937},
        {"tokenAttr", 0},
        {"dcaps", {"AT_CAP", "ST_CAP"}}};
    std::shared_ptr<NativeTokenInfoInner> p;
    from_json(j, p);
    ASSERT_EQ((p == nullptr), true);

    // APL wrong
    j = nlohmann::json{
        {"processName", "process6"},
        {"APL", -1}, {"version", 1},
        {"tokenId", 685266937}, {"tokenAttr", 0},
        {"dcaps", {"AT_CAP", "ST_CAP"}}};
    from_json(j, p);
    ASSERT_EQ((p == nullptr), true);

    // tokenId wrong
    j = nlohmann::json{
        {"processName", "process6"},
        {"APL", APL_SYSTEM_BASIC}, {"version", 1},
        {"tokenId", 0}, {"tokenAttr", 0},
        {"dcaps", {"AT_CAP", "ST_CAP"}}};
    from_json(j, p);
    ASSERT_EQ((p == nullptr), true);

    // process name empty
    j = nlohmann::json{
        {"processName", ""},
        {"APL", APL_SYSTEM_BASIC}, {"version", 1},
        {"tokenId", 685266937}, {"tokenAttr", 0},
        {"dcaps", {"AT_CAP", "ST_CAP"}}};
    from_json(j, p);
    ASSERT_EQ((p == nullptr), true);

    // process name too long
    std::string name(512, 'c');
    j = nlohmann::json{
        {"processName", name},
        {"APL", APL_SYSTEM_BASIC}, {"version", 1},
        {"tokenId", 685266937}, {"tokenAttr", 0},
        {"dcaps", {"AT_CAP", "ST_CAP"}}};
    from_json(j, p);
    ASSERT_EQ((p == nullptr), true);

    // lose process name
    j = nlohmann::json{
        {"APL", APL_SYSTEM_BASIC},
        {"version", 1}, {"tokenId", 685266937},
        {"tokenAttr", 0}, {"dcaps", {"AT_CAP", "ST_CAP"}}};
    from_json(j, p);
    ASSERT_EQ((p == nullptr), true);
}

/**
 * @tc.name: ProcessNativeTokenInfos001
 * @tc.desc: test add one native token
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(NativeTokenReceptorTest, ProcessNativeTokenInfos001, TestSize.Level1)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "test ProcessNativeTokenInfos001!");
    std::vector<std::shared_ptr<NativeTokenInfoInner>> tokenInfos;

    // test process one
    NativeTokenInfoBase info = {
        .apl = APL_NORMAL,
        .ver = 1,
        .processName = "ProcessNativeTokenInfos001",
        .dcap =  {"AT_CAP", "ST_CAP"},
        .tokenID = 0x28100000,
        .tokenAttr = 0
    };

    std::vector<PermissionStateFull> permStateList = {};
    std::shared_ptr<NativeTokenInfoInner> nativeToken = std::make_shared<NativeTokenInfoInner>(info, permStateList);
    tokenInfos.emplace_back(nativeToken);
    AccessTokenInfoManager::GetInstance().ProcessNativeTokenInfos(tokenInfos);
    NativeTokenInfoBase findInfo;
    int ret = AccessTokenInfoManager::GetInstance().GetNativeTokenInfo(info.tokenID, findInfo);
    ASSERT_EQ(ret, RET_SUCCESS);
    ASSERT_EQ(findInfo.apl, info.apl);
    ASSERT_EQ(findInfo.processName, info.processName);

    // wait fresh tokens to sql.
    sleep(3);

    // get sql data
    GenericValues conditionValue;
    std::vector<GenericValues> nativeTokenResults;
    AccessTokenDb::GetInstance().Find(AtmDataType::ACCESSTOKEN_NATIVE_INFO, conditionValue, nativeTokenResults);
    std::vector<GenericValues> permStateRes;
    AccessTokenDb::GetInstance().Find(AtmDataType::ACCESSTOKEN_PERMISSION_STATE, conditionValue, permStateRes);
    for (GenericValues nativeTokenValue : nativeTokenResults) {
        AccessTokenID tokenId = (AccessTokenID)nativeTokenValue.GetInt(TokenFiledConst::FIELD_TOKEN_ID);
        if (tokenId != info.tokenID) {
            continue;
        }
        GTEST_LOG_(INFO) << "apl " << nativeTokenValue.GetInt(TokenFiledConst::FIELD_APL);
        std::shared_ptr<NativeTokenInfoInner> native = std::make_shared<NativeTokenInfoInner>();
        ASSERT_NE(native, nullptr);
        ret = native->RestoreNativeTokenInfo(tokenId, nativeTokenValue, permStateRes);
        ASSERT_EQ(ret, RET_SUCCESS);
        ASSERT_EQ(native->GetTokenID(), info.tokenID);
        ASSERT_EQ(native->GetProcessName(), info.processName);
    }

    ret = AccessTokenInfoManager::GetInstance().RemoveNativeTokenInfo(info.tokenID);
    ASSERT_EQ(ret, RET_SUCCESS);
}

static void PermStateListSet(std::vector<PermissionStateFull> &permStateList)
{
    PermissionStateFull infoManagerTestState1 = {
        .permissionName = "ohos.permission.ACCELEROMETER",
        .isGeneral = true,
        .resDeviceID = {"local"},
        .grantStatus = {0},
        .grantFlags = {0}
    };

    PermissionStateFull infoManagerTestState2 = {
        .permissionName = "ohos.permission.MANAGE_USER_IDM",
        .isGeneral = true,
        .resDeviceID = {"device 1", "device 2"},
        .grantStatus = {0, 0},
        .grantFlags = {0, 2}
    };

    PermissionStateFull infoManagerTestState3 = {
        .permissionName = "ohos.permission.USER_TEAT",
        .isGeneral = true,
        .resDeviceID = {"device 1", "device 2"},
        .grantStatus = {0, 0},
        .grantFlags = {0, 2}
    };
    permStateList.emplace_back(infoManagerTestState1);
    permStateList.emplace_back(infoManagerTestState2);
    permStateList.emplace_back(infoManagerTestState3);
}

static void CompareGoalTokenInfo(const NativeTokenInfoBase &info)
{
    NativeTokenInfoBase findInfo;
    int ret = AccessTokenInfoManager::GetInstance().GetNativeTokenInfo(info.tokenID, findInfo);
    ASSERT_EQ(ret, RET_SUCCESS);
    ASSERT_EQ(findInfo.apl, info.apl);
    ASSERT_EQ(findInfo.processName, info.processName);
}

/**
 * @tc.name: ProcessNativeTokenInfos002
 * @tc.desc: test add two native tokens.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(NativeTokenReceptorTest, ProcessNativeTokenInfos002, TestSize.Level1)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "test ProcessNativeTokenInfos002!");
    std::vector<std::shared_ptr<NativeTokenInfoInner>> tokenInfos;
    NativeTokenInfoBase info1;
    info1.apl = APL_NORMAL;
    info1.ver = 1;
    info1.processName = "native_token_test1";
    info1.dcap =  {"AT_CAP", "ST_CAP"};
    info1.tokenID = 0x28100001;
    info1.tokenAttr = 0;

    NativeTokenInfoBase info2;
    info2.apl = APL_SYSTEM_BASIC;
    info2.ver = 1;
    info2.processName = "native_token_test2";
    info2.dcap =  {"AT_CAP", "ST_CAP"};
    info2.tokenID = 0x28100002;
    info2.tokenAttr = 0;

    std::vector<PermissionStateFull> permStateList;
    PermStateListSet(permStateList);
    std::shared_ptr<NativeTokenInfoInner> nativeToken1 = std::make_shared<NativeTokenInfoInner>(info1, permStateList);

    std::shared_ptr<PermissionPolicySet> permPolicySet =
        nativeToken1->GetNativeInfoPermissionPolicySet();
    GTEST_LOG_(INFO) <<"permPolicySet: " << permPolicySet;

    std::vector<PermissionStateFull> permList;
    permPolicySet->GetPermissionStateList(permList);
    for (const auto& perm : permList) {
        GTEST_LOG_(INFO) <<"perm.permissionName: " << perm.permissionName;
    }

    tokenInfos.emplace_back(nativeToken1);

    std::shared_ptr<NativeTokenInfoInner> nativeToken2 = std::make_shared<NativeTokenInfoInner>(info2, permStateList);
    tokenInfos.emplace_back(nativeToken2);

    AccessTokenInfoManager::GetInstance().ProcessNativeTokenInfos(tokenInfos);

    CompareGoalTokenInfo(info1);

    int ret = AccessTokenInfoManager::GetInstance().VerifyAccessToken(info1.tokenID, "ohos.permission.MANAGE_USER_IDM");
    ASSERT_EQ(ret, PERMISSION_GRANTED);
    ret = AccessTokenInfoManager::GetInstance().VerifyAccessToken(info1.tokenID, "ohos.permission.ACCELEROMETER");
    ASSERT_EQ(ret, PERMISSION_GRANTED);
    ret = AccessTokenInfoManager::GetInstance().VerifyAccessToken(info1.tokenID, "ohos.permission.DISCOVER_BLUETOOTH");
    ASSERT_EQ(ret, PERMISSION_DENIED);

    CompareGoalTokenInfo(info2);

    ret = AccessTokenInfoManager::GetInstance().RemoveNativeTokenInfo(info1.tokenID);
    ASSERT_EQ(ret, RET_SUCCESS);

    ret = AccessTokenInfoManager::GetInstance().VerifyAccessToken(info2.tokenID, "ohos.permission.MANAGE_USER_IDM");
    ASSERT_EQ(ret, PERMISSION_GRANTED);
    ret = AccessTokenInfoManager::GetInstance().VerifyAccessToken(info2.tokenID, "ohos.permission.ACCELEROMETER");
    ASSERT_EQ(ret, PERMISSION_GRANTED);

    ret = AccessTokenInfoManager::GetInstance().RemoveNativeTokenInfo(info2.tokenID);
    ASSERT_EQ(ret, RET_SUCCESS);
}

/**
 * @tc.name: ProcessNativeTokenInfos003
 * @tc.desc: test add nullptr tokenInfo.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(NativeTokenReceptorTest, ProcessNativeTokenInfos003, TestSize.Level1)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "test ProcessNativeTokenInfos003!");
    std::vector<std::shared_ptr<NativeTokenInfoInner>> tokenInfos;

    std::shared_ptr<NativeTokenInfoInner> nativeToken1 = std::make_shared<NativeTokenInfoInner>();
    tokenInfos.emplace_back(nativeToken1);
    AccessTokenInfoManager::GetInstance().ProcessNativeTokenInfos(tokenInfos);
    ASSERT_EQ(RET_SUCCESS, RET_SUCCESS);
}

/**
 * @tc.name: ProcessNativeTokenInfos004
 * @tc.desc: test add repeat id, but process doesn't
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(NativeTokenReceptorTest, ProcessNativeTokenInfos004, TestSize.Level1)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "test ProcessNativeTokenInfos004!");
    std::vector<std::shared_ptr<NativeTokenInfoInner>> tokenInfos;

    NativeTokenInfoBase info3 = {
        .apl = APL_NORMAL,
        .ver = 1,
        .processName = "native_token_test3",
        .dcap =  {"AT_CAP", "ST_CAP"},
        .tokenID = 0x28100003,
        .tokenAttr = 0
    };

    NativeTokenInfoBase info4 = {
        .apl = APL_NORMAL,
        .ver = 1,
        .processName = "native_token_test4",
        .dcap =  {"AT_CAP", "ST_CAP"},
        .tokenID = 0x28100003,
        .tokenAttr = 0
    };
    std::vector<PermissionStateFull> permStateList = {};
    std::shared_ptr<NativeTokenInfoInner> nativeToken3 = std::make_shared<NativeTokenInfoInner>(info3, permStateList);
    tokenInfos.emplace_back(nativeToken3);

    std::shared_ptr<NativeTokenInfoInner> nativeToken4 = std::make_shared<NativeTokenInfoInner>(info4, permStateList);
    tokenInfos.emplace_back(nativeToken4);

    AccessTokenInfoManager::GetInstance().ProcessNativeTokenInfos(tokenInfos);

    NativeTokenInfoBase findInfo;
    int ret = AccessTokenInfoManager::GetInstance().GetNativeTokenInfo(info3.tokenID, findInfo);
    ASSERT_EQ(ret, RET_SUCCESS);
    ASSERT_EQ(findInfo.apl, info3.apl);
    ASSERT_EQ(findInfo.processName, info3.processName);

    ret = AccessTokenInfoManager::GetInstance().RemoveNativeTokenInfo(info3.tokenID);
    ASSERT_EQ(ret, RET_SUCCESS);
}

/**
 * @tc.name: ProcessNativeTokenInfos005
 * @tc.desc: test add repeat process, but id doesn't
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(NativeTokenReceptorTest, ProcessNativeTokenInfos005, TestSize.Level1)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "test ProcessNativeTokenInfos005!");
    std::vector<std::shared_ptr<NativeTokenInfoInner>> tokenInfos;

    NativeTokenInfoBase info5 = {
        .apl = APL_NORMAL,
        .ver = 1,
        .processName = "native_token_test5",
        .dcap =  {"AT_CAP", "ST_CAP"},
        .tokenID = 0x28100005,
        .tokenAttr = 0
    };

    NativeTokenInfoBase info6 = {
        .apl = APL_NORMAL,
        .ver = 1,
        .processName = "native_token_test5",
        .dcap =  {"AT_CAP", "ST_CAP"},
        .tokenID = 0x28100006,
        .tokenAttr = 0
    };
    std::vector<PermissionStateFull> permStateList = {};
    std::shared_ptr<NativeTokenInfoInner> nativeToken5 = std::make_shared<NativeTokenInfoInner>(info5, permStateList);
    tokenInfos.emplace_back(nativeToken5);

    std::shared_ptr<NativeTokenInfoInner> nativeToken6 = std::make_shared<NativeTokenInfoInner>(info6, permStateList);
    tokenInfos.emplace_back(nativeToken6);

    AccessTokenInfoManager::GetInstance().ProcessNativeTokenInfos(tokenInfos);

    NativeTokenInfoBase findInfo;
    int ret = AccessTokenInfoManager::GetInstance().GetNativeTokenInfo(info5.tokenID, findInfo);
    ASSERT_EQ(ret, ERR_TOKENID_NOT_EXIST);

    ret = AccessTokenInfoManager::GetInstance().GetNativeTokenInfo(info6.tokenID, findInfo);
    ASSERT_EQ(ret, RET_SUCCESS);
    ASSERT_EQ(findInfo.apl, info6.apl);
    ASSERT_EQ(findInfo.processName, info6.processName);

    ret = AccessTokenInfoManager::GetInstance().RemoveNativeTokenInfo(info6.tokenID);
    ASSERT_EQ(ret, RET_SUCCESS);
}

/**
 * @tc.name: ProcessNativeTokenInfos006
 * @tc.desc: test add repeat process and id
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(NativeTokenReceptorTest, ProcessNativeTokenInfos006, TestSize.Level1)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "test ProcessNativeTokenInfos006!");
    std::vector<std::shared_ptr<NativeTokenInfoInner>> tokenInfos;

    NativeTokenInfoBase info7 = {
        .apl = APL_NORMAL,
        .ver = 1,
        .processName = "native_token_test7",
        .dcap =  {"AT_CAP", "ST_CAP"},
        .tokenID = 0x28100007,
        .tokenAttr = 0
    };

    NativeTokenInfoBase info8 = {
        .apl = APL_SYSTEM_BASIC,
        .ver = 1,
        .processName = "native_token_test7",
        .dcap =  {"AT_CAP"},
        .tokenID = 0x28100007,
        .tokenAttr = 0
    };
    std::vector<PermissionStateFull> permStateList = {};
    std::shared_ptr<NativeTokenInfoInner> nativeToken7 = std::make_shared<NativeTokenInfoInner>(info7, permStateList);
    tokenInfos.emplace_back(nativeToken7);

    std::shared_ptr<NativeTokenInfoInner> nativeToken8 = std::make_shared<NativeTokenInfoInner>(info8, permStateList);
    tokenInfos.emplace_back(nativeToken8);

    AccessTokenInfoManager::GetInstance().ProcessNativeTokenInfos(tokenInfos);

    NativeTokenInfoBase findInfo;
    int ret = AccessTokenInfoManager::GetInstance().GetNativeTokenInfo(info7.tokenID, findInfo);
    ASSERT_EQ(ret, RET_SUCCESS);
    ASSERT_EQ(findInfo.apl, info8.apl);
    ASSERT_EQ(findInfo.processName, info8.processName);

    ret = AccessTokenInfoManager::GetInstance().RemoveNativeTokenInfo(info8.tokenID);
    ASSERT_EQ(ret, RET_SUCCESS);
}

/**
 * @tc.name: init001
 * @tc.desc: test get native cfg
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(NativeTokenReceptorTest, init001, TestSize.Level1)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "test init001!");

    const char *dcaps[1];
    dcaps[0] = "AT_CAP_01";
    int dcapNum = 1;
    const char *perms[2];
    perms[0] = "ohos.permission.test1";
    perms[1] = "ohos.permission.test2";
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = dcapNum,
        .permsNum = 2,
        .aclsNum = 0,
        .dcaps = dcaps,
        .perms = perms,
        .acls = nullptr,
        .processName = "native_token_test7",
        .aplStr = "system_core",
    };
    uint64_t tokenId = ::GetAccessTokenId(&infoInstance);
    ASSERT_NE(tokenId, INVALID_TOKENID);

    NativeTokenReceptor::GetInstance().Init();
    NativeTokenInfoBase findInfo;
    int ret = AccessTokenInfoManager::GetInstance().GetNativeTokenInfo(tokenId, findInfo);
    ASSERT_EQ(ret, RET_SUCCESS);
    ASSERT_EQ(findInfo.processName, infoInstance.processName);

    ret = AccessTokenInfoManager::GetInstance().RemoveNativeTokenInfo(tokenId);
    ASSERT_EQ(ret, RET_SUCCESS);
}