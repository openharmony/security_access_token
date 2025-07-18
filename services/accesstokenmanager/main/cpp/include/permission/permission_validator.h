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

#ifndef PERMISSION_VALIDATOR_H
#define PERMISSION_VALIDATOR_H
#include "permission_def.h"
#include "permission_status.h"

namespace OHOS {
namespace Security {
namespace AccessToken {
struct PermissionValidator final {
public:
    PermissionValidator() {}
    ~PermissionValidator() {}

    static bool IsPermissionAvailable(ATokenTypeEnum tokenType, const std::string& permissionName);
    static bool IsPermissionNameValid(const std::string& permissionName);
    static bool IsUserIdValid(const int32_t userID);
    static bool IsToggleStatusValid(const uint32_t status);
    static bool IsPermissionFlagValid(uint32_t flag);
    static bool IsPermissionFlagValidForAdmin(uint32_t flag);
    static bool IsPermissionDefValid(const PermissionDef& permDef);
    static bool IsPermissionStateValid(const PermissionStatus& permState);
    static void FilterInvalidPermissionDef(
        const std::vector<PermissionDef>& permList, std::vector<PermissionDef>& result);
    static void FilterInvalidPermissionState(ATokenTypeEnum tokenType, bool doPermAvailableCheck,
        const std::vector<PermissionStatus>& permList, std::vector<PermissionStatus>& result);
    static bool IsGrantModeValid(int grantMode);
    static bool IsGrantStatusValid(int grantStatus);
};
}  // namespace AccessToken
}  // namespace Security
}  // namespace OHOS
#endif  // PERMISSION_VALIDATOR_H

