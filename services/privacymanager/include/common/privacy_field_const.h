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
    const static char* FIELD_TOKEN_ID;
    const static char* FIELD_DEVICE_ID;
    const static char* FIELD_OP_CODE;
    const static char* FIELD_STATUS;
    const static char* FIELD_TIMESTAMP;
    const static char* FIELD_ACCESS_DURATION;
    const static char* FIELD_ACCESS_COUNT;
    const static char* FIELD_REJECT_COUNT;

    const static char* FIELD_TIMESTAMP_BEGIN;
    const static char* FIELD_TIMESTAMP_END;
    const static char* FIELD_FLAG;
};
} // namespace AccessToken
} // namespace Security
} // namespace OHOS
#endif // PRIVACY_FIELD_CONST_H
