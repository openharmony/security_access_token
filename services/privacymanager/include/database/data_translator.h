/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef DATA_TRANSLATOR_H
#define DATA_TRANSLATOR_H

#include <string>

#include "permission_used_request.h"
#include "permission_used_result.h"
#include "generic_values.h"

namespace OHOS {
namespace Security {
namespace AccessToken {
class DataTranslator final {
public:
    static int32_t TranslationIntoGenericValues(const PermissionUsedRequest& request,
    GenericValues& visitorGenericValues, GenericValues& andGenericValues, GenericValues& orGenericValues);
    static int32_t TranslationGenericValuesIntoPermissionUsedRecord(
        const GenericValues& inGenericValues, PermissionUsedRecord& permissionRecord);
};
} // namespace AccessToken
} // namespace Security
} // namespace OHOS
#endif // DATA_TRANSLATOR_H
