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

/**
 * @addtogroup Privacy
 * @{
 *
 * @brief Provides sensitive data access management.
 *
 * @since 9.0
 * @version 9.0
 */

/**
 * @file permission_used_type_info.h
 *
 * @brief Declares PermissionUsedTypeInfo struct.
 *
 * @since 12.0
 * @version 12.0
 */

#ifndef SECURITY_ACCESSTOKEN_PERMISSION_USED_TYPE_INFO_H
#define SECURITY_ACCESSTOKEN_PERMISSION_USED_TYPE_INFO_H

#include <string>
#include "access_token.h"
#include "permission_used_type.h"

namespace OHOS {
namespace Security {
namespace AccessToken {
/**
 * @brief add permission used type
 */
struct PermissionUsedTypeInfo {
    AccessTokenID tokenId;
    std::string permissionName;
    /** for input parameter only **/
    int32_t pid;
    /** enum PermissionUsedType, see permission_used_type.h */
    PermissionUsedType type = NORMAL_TYPE;
};
} // namespace AccessToken
} // namespace Security
} // namespace OHOS
#endif // SECURITY_ACCESSTOKEN_PERMISSION_USED_TYPE_INFO_H