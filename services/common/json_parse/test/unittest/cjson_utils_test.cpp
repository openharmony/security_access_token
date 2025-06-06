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

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include "cjson_utils.h"

using namespace testing::ext;

namespace OHOS {
namespace Security {
namespace AccessToken {

class CJsonUtilsTest : public testing::Test  {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();

    void SetUp();
    void TearDown();
};

void CJsonUtilsTest::SetUpTestCase() {}
void CJsonUtilsTest::TearDownTestCase() {}
void CJsonUtilsTest::SetUp() {}
void CJsonUtilsTest::TearDown() {}

/*
 * @tc.name: CreateJsonFromString
 * @tc.desc: CreateJsonFromString
 * @tc.type: FUNC
 * @tc.require: TDD coverage
 */
HWTEST_F(CJsonUtilsTest, CreateJsonFromStringTest001, TestSize.Level3)
{
    std::string test;
    EXPECT_EQ(nullptr, CreateJsonFromString(test));
}

/*
 * @tc.name: PackJsonToString
 * @tc.desc: PackJsonToString
 * @tc.type: FUNC
 * @tc.require: TDD coverage
 */
HWTEST_F(CJsonUtilsTest, PackJsonToStringTest001, TestSize.Level3)
{
    std::string res = PackJsonToString(nullptr);
    EXPECT_EQ(res.size(), 0);

    FreeJsonString(nullptr);
}

/*
 * @tc.name: GetObjFromJson
 * @tc.desc: GetObjFromJson
 * @tc.type: FUNC
 * @tc.require: TDD coverage
 */
HWTEST_F(CJsonUtilsTest, GetObjFromJsonTest001, TestSize.Level3)
{
    std::string test;
    EXPECT_EQ(nullptr, GetObjFromJson(nullptr, test));

    test = "test1";
    CJsonUnique jsonInner = CreateJson();
    ASSERT_EQ(true, AddStringToJson(jsonInner, "test0", "0"));
    EXPECT_EQ(nullptr, GetObjFromJson(jsonInner, test));

    test = "test0";
    EXPECT_EQ(nullptr, GetObjFromJson(jsonInner, test));
}

/*
 * @tc.name: GetArrayFromJson
 * @tc.desc: GetArrayFromJson
 * @tc.type: FUNC
 * @tc.require: TDD coverage
 */
HWTEST_F(CJsonUtilsTest, GetArrayFromJsonTest001, TestSize.Level3)
{
    std::string test;
    EXPECT_EQ(nullptr, GetArrayFromJson(nullptr, test));

    test = "test1";
    CJsonUnique jsonInner = CreateJson();
    ASSERT_EQ(true, AddStringToJson(jsonInner, "test0", "0"));
    EXPECT_EQ(nullptr, GetArrayFromJson(jsonInner, test));

    test = "test0";
    EXPECT_EQ(nullptr, GetArrayFromJson(jsonInner, test));
}

/*
 * @tc.name: GetStringFromJson
 * @tc.desc: GetStringFromJson
 * @tc.type: FUNC
 * @tc.require: TDD coverage
 */
HWTEST_F(CJsonUtilsTest, GetStringFromJsonTest001, TestSize.Level3)
{
    std::string test;
    std::string res;
    EXPECT_EQ(false, GetStringFromJson(nullptr, test, res));

    CJsonUnique jsonInner = CreateJson();
    ASSERT_EQ(true, AddStringToJson(jsonInner, "test0", "0"));
    EXPECT_EQ(false, GetStringFromJson(jsonInner.get(), test, res));

    test = "test1";
    EXPECT_EQ(false, GetStringFromJson(jsonInner.get(), test, res));

    CJsonUnique jsonArray = CreateJsonArray();
    ASSERT_EQ(true, AddObjToJson(jsonArray, "test1", jsonInner));
    EXPECT_EQ(false, GetStringFromJson(jsonArray.get(), test, res));
}

/*
 * @tc.name: GetIntFromJson
 * @tc.desc: GetIntFromJson
 * @tc.type: FUNC
 * @tc.require: TDD coverage
 */
HWTEST_F(CJsonUtilsTest, GetIntFromJsonTest001, TestSize.Level3)
{
    std::string test;
    int32_t res;
    EXPECT_EQ(false, GetIntFromJson(nullptr, test, res));

    test = "test1";
    CJsonUnique jsonInner = CreateJson();
    ASSERT_EQ(true, AddStringToJson(jsonInner, "test0", "abc"));
    EXPECT_EQ(false, GetIntFromJson(jsonInner, test, res));

    test = "test0";
    EXPECT_EQ(false, GetIntFromJson(jsonInner, test, res));
}

/*
 * @tc.name: GetUnsignedIntFromJson
 * @tc.desc: GetUnsignedIntFromJson
 * @tc.type: FUNC
 * @tc.require: TDD coverage
 */
HWTEST_F(CJsonUtilsTest, GetUnsignedIntFromJsonTest001, TestSize.Level3)
{
    std::string test;
    uint32_t res;
    EXPECT_EQ(false, GetUnsignedIntFromJson(nullptr, test, res));

    test = "test1";
    CJsonUnique jsonInner = CreateJson();
    ASSERT_EQ(true, AddStringToJson(jsonInner, "test0", "abc"));
    EXPECT_EQ(false, GetUnsignedIntFromJson(jsonInner, test, res));

    test = "test0";
    EXPECT_EQ(false, GetUnsignedIntFromJson(jsonInner, test, res));
}

/*
 * @tc.name: GetBoolFromJson
 * @tc.desc: GetBoolFromJson
 * @tc.type: FUNC
 * @tc.require: TDD coverage
 */
HWTEST_F(CJsonUtilsTest, GetBoolFromJsonTest001, TestSize.Level3)
{
    std::string test;
    bool res;
    EXPECT_EQ(false, GetBoolFromJson(nullptr, test, res));

    test = "test1";
    CJsonUnique jsonInner = CreateJson();
    ASSERT_EQ(true, AddStringToJson(jsonInner, "test0", "0"));
    EXPECT_EQ(false, GetBoolFromJson(jsonInner, test, res));

    test = "test0";
    EXPECT_EQ(false, GetBoolFromJson(jsonInner, test, res));
}

/*
 * @tc.name: GetBoolFromJson
 * @tc.desc: GetBoolFromJson
 * @tc.type: FUNC
 * @tc.require: TDD coverage
 */
HWTEST_F(CJsonUtilsTest, GetBoolFromJsonTest002, TestSize.Level3)
{
    std::string test = "test1";
    bool res;
    CJsonUnique jsonInner = CreateJson();
    ASSERT_EQ(true, AddBoolToJson(jsonInner, test, true));

    EXPECT_EQ(true, GetBoolFromJson(jsonInner, test, res));
    EXPECT_EQ(res, true);
}

/*
 * @tc.name: AddObjToJson
 * @tc.desc: AddObjToJson
 * @tc.type: FUNC
 * @tc.require: TDD coverage
 */
HWTEST_F(CJsonUtilsTest, AddObjToJsonTest001, TestSize.Level3)
{
    ASSERT_EQ(false, AddObjToJson(nullptr, "", nullptr));
    std::string test = "test1";
    ASSERT_EQ(false, AddObjToJson(nullptr, test, nullptr));

    CJsonUnique jsonInner = CreateJson();
    ASSERT_EQ(true, AddStringToJson(jsonInner, "test0", "0"));
    ASSERT_EQ(true, AddStringToJson(jsonInner, "test1", "1"));

    CJsonUnique jsonArray = CreateJsonArray();
    ASSERT_EQ(true, AddObjToJson(jsonArray, "test1", jsonInner));
    ASSERT_EQ(true, AddObjToJson(jsonArray, "test1", jsonInner));
}

/*
 * @tc.name: AddObjToArray
 * @tc.desc: AddObjToArray
 * @tc.type: FUNC
 * @tc.require: TDD coverage
 */
HWTEST_F(CJsonUtilsTest, AddObjToArrayTest001, TestSize.Level3)
{
    ASSERT_EQ(false, AddObjToArray(nullptr, nullptr));

    CJsonUnique jsonInner = CreateJson();
    ASSERT_EQ(false, AddObjToArray(nullptr, jsonInner.get()));
}

/*
 * @tc.name: AddStringToJson
 * @tc.desc: AddStringToJson
 * @tc.type: FUNC
 * @tc.require: TDD coverage
 */
HWTEST_F(CJsonUtilsTest, AddStringToJsonTest001, TestSize.Level3)
{
    ASSERT_EQ(false, AddStringToJson(nullptr, "", ""));
    ASSERT_EQ(false, AddStringToJson(nullptr, "test0", "test0"));

    CJsonUnique jsonInner = CreateJson();
    ASSERT_EQ(true, AddStringToJson(jsonInner, "test0", "test0"));
    // twice
    ASSERT_EQ(true, AddStringToJson(jsonInner, "test0", "test0"));
}

/*
 * @tc.name: AddBoolToJson
 * @tc.desc: AddBoolToJson
 * @tc.type: FUNC
 * @tc.require: TDD coverage
 */
HWTEST_F(CJsonUtilsTest, AddBoolToJsonTest001, TestSize.Level3)
{
    ASSERT_EQ(false, AddBoolToJson(nullptr, "", true));
    ASSERT_EQ(false, AddBoolToJson(nullptr, "test0", true));

    CJsonUnique jsonInner = CreateJson();
    ASSERT_EQ(true, AddBoolToJson(jsonInner, "test0", true));
    // twice
    ASSERT_EQ(true, AddBoolToJson(jsonInner, "test0", true));
}

/*
 * @tc.name: AddIntToJson
 * @tc.desc: AddIntToJson
 * @tc.type: FUNC
 * @tc.require: TDD coverage
 */
HWTEST_F(CJsonUtilsTest, AddIntToJsonTest001, TestSize.Level3)
{
    ASSERT_EQ(false, AddIntToJson(nullptr, "", 0));
    ASSERT_EQ(false, AddIntToJson(nullptr, "test0", 0));

    CJsonUnique jsonInner = CreateJson();
    ASSERT_EQ(true, AddIntToJson(jsonInner, "test0", 0));
    // twice
    ASSERT_EQ(true, AddIntToJson(jsonInner, "test0", 0));
}

/*
 * @tc.name: AddUnsignedIntToJson
 * @tc.desc: AddUnsignedIntToJson
 * @tc.type: FUNC
 * @tc.require: TDD coverage
 */
HWTEST_F(CJsonUtilsTest, AddUnsignedIntToJsonTest001, TestSize.Level3)
{
    ASSERT_EQ(false, AddUnsignedIntToJson(nullptr, "", 0));
    ASSERT_EQ(false, AddUnsignedIntToJson(nullptr, "test0", 0));

    CJsonUnique jsonInner = CreateJson();
    ASSERT_EQ(true, AddUnsignedIntToJson(jsonInner, "test0", 0));
    // twice
    ASSERT_EQ(true, AddUnsignedIntToJson(jsonInner, "test0", 0));
}
} // namespace AccessToken
} // namespace Security
} // namespace OHOS
