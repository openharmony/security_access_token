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

#ifndef PRIVACY_FIELD_CONST_H
#define PRIVACY_FIELD_CONST_H

#include <string>

namespace OHOS {
namespace Security {
namespace AccessToken {
class PrivacyFiledConst {
public:
    const static std::string FIELD_USER_ID;
    const static std::string FIELD_TOKEN_ID;
    const static std::string FIELD_DEVICE_ID;
    const static std::string FIELD_OP_CODE;
    const static std::string FIELD_STATUS;
    const static std::string FIELD_TIMESTAMP;
    const static std::string FIELD_ACCESS_DURATION;
    const static std::string FIELD_ACCESS_COUNT;
    const static std::string FIELD_REJECT_COUNT;

    const static std::string FIELD_TIMESTAMP_BEGIN;
    const static std::string FIELD_TIMESTAMP_END;
    const static std::string FIELD_FLAG;
    const static std::string FIELD_LOCKSCREEN_STATUS;

    const static std::string FIELD_PERMISSION_CODE;
    const static std::string FIELD_USED_TYPE;
};
} // namespace AccessToken
} // namespace Security
} // namespace OHOS
#endif // PRIVACY_FIELD_CONST_H
