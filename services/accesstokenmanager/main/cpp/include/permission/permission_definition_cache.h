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

#ifndef PERMISSION_DEFINITION_CACHE_H
#define PERMISSION_DEFINITION_CACHE_H

#include <map>
#include <vector>

#include "data_translator.h"
#include "permission_def.h"

#include "rwlock.h"
#include "nocopyable.h"

namespace OHOS {
namespace Security {
namespace AccessToken {
class PermissionDefinitionCache final {
public:
    static PermissionDefinitionCache& GetInstance();

    virtual ~PermissionDefinitionCache();

    bool Insert(const PermissionDef& info, AccessTokenID tokenId);

    bool Update(const PermissionDef& info, AccessTokenID tokenId);

    void DeleteByToken(AccessTokenID tokenId);

    int FindByPermissionName(const std::string& permissionName, PermissionDef& info);

    bool IsSystemGrantedPermission(const std::string& permissionName);

    bool IsUserGrantedPermission(const std::string& permissionName);

    bool HasDefinition(const std::string& permissionName);

    bool IsHapPermissionDefEmpty();

    void StorePermissionDef(std::vector<GenericValues>& valueList);

    void StorePermissionDef(AccessTokenID tokenID, std::vector<GenericValues>& valueList);

    void GetDefPermissionsByTokenId(std::vector<PermissionDef>& permList, AccessTokenID tokenId);

    int32_t RestorePermDefInfo(std::vector<GenericValues>& permDefRes);

    bool HasHapPermissionDefinitionForHap(const std::string& permissionName);

    uint32_t GetDefPermissionsSize();

private:
    PermissionDefinitionCache();

    bool IsGrantedModeEqualInner(const std::string& permissionName, int grantMode) const;

    DISALLOW_COPY_AND_MOVE(PermissionDefinitionCache);

    /**
     * key: the permission name.
     * value: the object of PermissionDefData.
     */
    std::map<std::string, PermissionDefData> permissionDefinitionMap_;

    bool hasHapPermissionDefinition_ = false;

    OHOS::Utils::RWLock cacheLock_;
};
} // namespace AccessToken
} // namespace Security
} // namespace OHOS
#endif // PERMISSION_DEFINITION_CACHE_H
