/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
 * @addtogroup AccessToken
 * @{
 *
 * @brief Provides permission management interfaces.
 *
 * Provides tokenID-based application permission verification mechanism.
 * When an application accesses sensitive data or APIs, this module can check
 * whether the application has the corresponding permission. Allows applications
 * to query their access token information or APL levcels based on token IDs.
 *
 * @since 7.0
 * @version 7.0
 */

/**
 * @file hap_token_info.h
 *
 * @brief Declares hap token infos.
 *
 * @since 7.0
 * @version 7.0
 */

#ifndef ACCESSTOKEN_HAP_TOKEN_INFO_H
#define ACCESSTOKEN_HAP_TOKEN_INFO_H

#include "access_token.h"
#include "permission_def.h"
#include "permission_state_full.h"
#include "permission_status.h"
#include <map>
#include <string>
#include <vector>

namespace OHOS {
namespace Security {
namespace AccessToken {
/**
 * @brief Declares hap info params class
 */
class HapInfoParams final {
public:
    int userID;
    std::string bundleName;
    /** instance index */
    int instIndex;
    /**
     * dlp type, for details about the valid values,
     * see the definition of HapDlpType in the access_token.h file.
     */
    int dlpType;
    std::string appIDDesc;
    /** which version of the SDK is used to develop the hap */
    int32_t apiVersion;
    /** indicates whether the hap is a system app */
    bool isSystemApp;
    /* app type */
    std::string appDistributionType;
    bool isRestore = false;
    AccessTokenID tokenID = INVALID_TOKENID;
    /** Whether hap is a atomic-service */
    bool isAtomicService = false;
};

/**
 * @brief Declares hap info params class
 */
class UpdateHapInfoParams final {
public:
    std::string appIDDesc;
    /** which version of the SDK is used to develop the hap */
    int32_t apiVersion;
    /** indicates whether the hap is a system app */
    bool isSystemApp;
    /* app type */
    std::string appDistributionType;
    /** Whether hap is a atomic-service */
    bool isAtomicService = false;
    /** ota refresh app */
    bool dataRefresh = false;
};

/**
 * @brief Declares hap token info class
 */
class HapTokenInfo final {
public:
    char ver;
    int userID = 0;
    std::string bundleName;
    /** which version of the SDK is used to develop this hap */
    int32_t apiVersion;
    /** instance index */
    int instIndex = 0;
    /**
     * dlp type, for details about the valid values,
     * see the definition of HapDlpType in the access_token.h file.
     */
    int dlpType;
    AccessTokenID tokenID;
    /** token attribute */
    AccessTokenAttr tokenAttr;
};

/**
 * @brief Declares hap token info for distributed synchronize class
 */
class HapTokenInfoForSync final {
public:
    /** hap token info */
    HapTokenInfo baseInfo;
    /** permission state list */
    std::vector<PermissionStatus> permStateList;
};

class HapTokenInfoExt final {
public:
    /** hap token info */
    HapTokenInfo baseInfo;
    /** hap app id */
    std::string appID;
};

/**
 * @brief Declares hap base token info class
 */
class HapBaseInfo final {
public:
    int32_t userID;
    std::string bundleName = "";
    /** instance index */
    int32_t instIndex = 0;
};

/**
 * @brief Pre-authorization token info class
 */
class PreAuthorizationInfo final {
public:
    std::string permissionName;
    /** Whether the pre-authorization is non-cancelable */
    bool userCancelable = false;
};

/**
 * @brief Declares hap policy params class
 */
class HapPolicyParams final {
public:
    /**
     * apl level, for details about the valid values,
     * see the definition of ATokenAplEnum in the access_token.h file.
     */
    ATokenAplEnum apl;
    std::string domain;
    std::vector<PermissionDef> permList;
    std::vector<PermissionStateFull> permStateList;
    std::vector<std::string> aclRequestedList;
    std::vector<PreAuthorizationInfo> preAuthorizationInfo;
    HapPolicyCheckIgnore checkIgnore = HapPolicyCheckIgnore::NONE;
    std::map<std::string, std::string> aclExtendedMap;
};

/**
 * @brief Declares the result after failing to update or install hap
 */
class PermissionInfoCheckResult final {
public:
    std::string permissionName;
    PermissionRulesEnum rule;
};

class HapInfoCheckResult final {
public:
    /**
     * permission detail after failing to install or update hap
     */
    PermissionInfoCheckResult permCheckResult;
};

/**
 * @brief Declares hap policy params class
 */
class HapPolicy final {
public:
    /**
     * apl level, for details about the valid values,
     * see the definition of ATokenAplEnum in the access_token.h file.
     */
    ATokenAplEnum apl;
    std::string domain;
    std::vector<PermissionDef> permList;
    std::vector<PermissionStatus> permStateList;
    std::vector<std::string> aclRequestedList;
    std::vector<PreAuthorizationInfo> preAuthorizationInfo;
    HapPolicyCheckIgnore checkIgnore = HapPolicyCheckIgnore::NONE;
    std::map<std::string, std::string> aclExtendedMap;
};

/**
 * @brief Declares hap Init info class
 */
class HapInitInfo final {
public:
    HapInfoParams installInfo;
    UpdateHapInfoParams updateInfo;
    HapPolicy policy;
    std::string bundleName = "";
    AccessTokenID tokenID = 0;
    bool isUpdate = false;
};

/**
 * @brief Declares permission with value
 */
class PermissionWithValue final {
public:
    std::string permissionName;
    std::string value;
};
} // namespace AccessToken
} // namespace Security
} // namespace OHOS
#endif // ACCESSTOKEN_HAP_TOKEN_INFO_H
