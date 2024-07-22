/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "permission_map.h"

#include <map>
#include <string>
#include <vector>

namespace OHOS {
namespace Security {
namespace AccessToken {
const static uint32_t MAX_PERM_SIZE = 2048;
/*
Permission code must be a number between 0 and OP_MAX.
The number must be added sequentially.
*/
const static std::vector<std::pair<std::string, bool>> g_permMap = {
    /* first: permission name, second: true-user_grant/false-system_grant */
    {"ohos.permission.ANSWER_CALL",             true},
    {"ohos.permission.READ_CALENDAR",           true},
    {"ohos.permission.READ_CALL_LOG",           true},
    {"ohos.permission.READ_CELL_MESSAGES",      true},
    {"ohos.permission.READ_CONTACTS",           true},
    {"ohos.permission.READ_MESSAGES",           true},
    {"ohos.permission.RECEIVE_MMS",             true},
    {"ohos.permission.RECEIVE_SMS",             true},
    {"ohos.permission.RECEIVE_WAP_MESSAGES",    true},
    {"ohos.permission.MICROPHONE",              true},
    {"ohos.permission.SEND_MESSAGES",           true},
    {"ohos.permission.WRITE_CALENDAR",          true},
    {"ohos.permission.WRITE_CALL_LOG",          true},
    {"ohos.permission.WRITE_CONTACTS",          true},
    {"ohos.permission.DISTRIBUTED_DATASYNC",    true},
    {"ohos.permission.MANAGE_VOICEMAIL",        true},
    {"ohos.permission.LOCATION_IN_BACKGROUND",  true},
    {"ohos.permission.LOCATION",                true},
    {"ohos.permission.APPROXIMATELY_LOCATION",  true},
    {"ohos.permission.MEDIA_LOCATION",          true},
    {"ohos.permission.CAMERA",                  true},
    {"ohos.permission.READ_MEDIA",              true},
    {"ohos.permission.WRITE_MEDIA",             true},
    {"ohos.permission.ACTIVITY_MOTION",         true},
    {"ohos.permission.READ_HEALTH_DATA",        true},
    {"ohos.permission.READ_IMAGEVIDEO",         true},
    {"ohos.permission.READ_AUDIO",              true},
    {"ohos.permission.READ_DOCUMENT",           true},
    {"ohos.permission.WRITE_IMAGEVIDEO",        true},
    {"ohos.permission.WRITE_AUDIO",             true},
    {"ohos.permission.WRITE_DOCUMENT",          true},
    {"ohos.permission.READ_WHOLE_CALENDAR",     true},
    {"ohos.permission.WRITE_WHOLE_CALENDAR",    true},
    {"ohos.permission.APP_TRACKING_CONSENT",    true},
    {"ohos.permission.GET_INSTALLED_BUNDLE_LIST", true},
    {"ohos.permission.ACCESS_BLUETOOTH",        true},
    {"ohos.permission.READ_PASTEBOARD",         true},
    {"ohos.permission.READ_WRITE_DOWNLOAD_DIRECTORY", true},
    {"ohos.permission.READ_WRITE_DOCUMENTS_DIRECTORY", true},
    {"ohos.permission.READ_WRITE_DESKTOP_DIRECTORY", true},
    {"ohos.permission.USE_BLUETOOTH",           false},
    {"ohos.permission.DISCOVER_BLUETOOTH",      false},
    {"ohos.permission.MANAGE_BLUETOOTH",        false},
    {"ohos.permission.GET_BLUETOOTH_LOCAL_MAC", false},
    {"ohos.permission.GET_BLUETOOTH_PEERS_MAC", false},
    {"ohos.permission.INTERNET",                false},
    {"ohos.permission.MODIFY_AUDIO_SETTINGS",   false},
    {"ohos.permission.ACCESS_NOTIFICATION_POLICY", false},
    {"ohos.permission.GET_TELEPHONY_STATE",     false},
    {"ohos.permission.GET_PHONE_NUMBERS",       false},
    {"ohos.permission.DISTRIBUTED_SOFTBUS_CENTER", false},
    {"ohos.permission.REQUIRE_FORM",            false},
    {"ohos.permission.AGENT_REQUIRE_FORM",      false},
    {"ohos.permission.GET_NETWORK_INFO",        false},
    {"ohos.permission.PLACE_CALL",              false},
    {"ohos.permission.SET_NETWORK_INFO",        false},
    {"ohos.permission.REMOVE_CACHE_FILES",      false},
    {"ohos.permission.REBOOT",                  false},
    {"ohos.permission.RUNNING_LOCK",            false},
    {"ohos.permission.SET_TIME",                false},
    {"ohos.permission.SET_TIME_ZONE",           false},
    {"ohos.permission.DOWNLOAD_SESSION_MANAGER", false},
    {"ohos.permission.COMMONEVENT_STICKY",      false},
    {"ohos.permission.SYSTEM_FLOAT_WINDOW",     false},
    {"ohos.permission.PRIVACY_WINDOW",          false},
    {"ohos.permission.POWER_MANAGER",           false},
    {"ohos.permission.REFRESH_USER_ACTION",     false},
    {"ohos.permission.POWER_OPTIMIZATION",      false},
    {"ohos.permission.REBOOT_RECOVERY",         false},
    {"ohos.permission.MANAGE_LOCAL_ACCOUNTS",   false},
    {"ohos.permission.INTERACT_ACROSS_LOCAL_ACCOUNTS", false},
    {"ohos.permission.VIBRATE",                 false},
    {"ohos.permission.SYSTEM_LIGHT_CONTROL",    false},
    {"ohos.permission.CONNECT_IME_ABILITY",     false},
    {"ohos.permission.CONNECT_SCREEN_SAVER_ABILITY", false},
    {"ohos.permission.READ_SCREEN_SAVER",       false},
    {"ohos.permission.WRITE_SCREEN_SAVER",      false},
    {"ohos.permission.SET_WALLPAPER",           false},
    {"ohos.permission.GET_WALLPAPER",           false},
    {"ohos.permission.CHANGE_ABILITY_ENABLED_STATE", false},
    {"ohos.permission.ACCESS_MISSIONS",         false},
    {"ohos.permission.CLEAN_BACKGROUND_PROCESSES", false},
    {"ohos.permission.KEEP_BACKGROUND_RUNNING", false},
    {"ohos.permission.UPDATE_CONFIGURATION",    false},
    {"ohos.permission.UPDATE_SYSTEM",           false},
    {"ohos.permission.FACTORY_RESET",           false},
    {"ohos.permission.ASSIST_DEVICE_UPDATE",    false},
    {"ohos.permission.RECEIVE_UPDATE_MESSAGE",    false},
    {"ohos.permission.UPDATE_MIGRATE",          false},
    {"ohos.permission.GRANT_SENSITIVE_PERMISSIONS", false},
    {"ohos.permission.REVOKE_SENSITIVE_PERMISSIONS", false},
    {"ohos.permission.GET_SENSITIVE_PERMISSIONS", false},
    {"ohos.permission.INTERACT_ACROSS_LOCAL_ACCOUNTS_EXTENSION", false},
    {"ohos.permission.LISTEN_BUNDLE_CHANGE",    false},
    {"ohos.permission.GET_BUNDLE_INFO",         false},
    {"ohos.permission.ACCELEROMETER",           false},
    {"ohos.permission.GYROSCOPE",               false},
    {"ohos.permission.GET_BUNDLE_INFO_PRIVILEGED", false},
    {"ohos.permission.INSTALL_BUNDLE",          false},
    {"ohos.permission.MANAGE_SHORTCUTS",        false},
    {"ohos.permission.radio.ACCESS_FM_AM",      false},
    {"ohos.permission.SET_TELEPHONY_STATE",     false},
    {"ohos.permission.ACCESS_BOOSTER_SERVICE", false},
    {"ohos.permission.START_ABILIIES_FROM_BACKGROUND", false},
    {"ohos.permission.START_ABILITIES_FROM_BACKGROUND", false},
    {"ohos.permission.BUNDLE_ACTIVE_INFO",      false},
    {"ohos.permission.START_INVISIBLE_ABILITY", false},
    {"ohos.permission.sec.ACCESS_UDID",         false},
    {"ohos.permission.LAUNCH_DATA_PRIVACY_CENTER", false},
    {"ohos.permission.MANAGE_MEDIA_RESOURCES",  false},
    {"ohos.permission.PUBLISH_AGENT_REMINDER",  false},
    {"ohos.permission.CONTROL_TASK_SYNC_ANIMATOR", false},
    {"ohos.permission.INPUT_MONITORING",        false},
    {"ohos.permission.MANAGE_MISSIONS",         false},
    {"ohos.permission.NOTIFICATION_CONTROLLER", false},
    {"ohos.permission.CONNECTIVITY_INTERNAL",   false},
    {"ohos.permission.MANAGE_NET_STRATEGY",     false},
    {"ohos.permission.GET_NETWORK_STATS",       false},
    {"ohos.permission.MANAGE_VPN",              false},
    {"ohos.permission.SET_ABILITY_CONTROLLER",  false},
    {"ohos.permission.USE_USER_IDM",            false},
    {"ohos.permission.MANAGE_USER_IDM",         false},
    {"ohos.permission.NETSYS_INTERNAL",         false},
    {"ohos.permission.ACCESS_BIOMETRIC",        false},
    {"ohos.permission.ACCESS_USER_AUTH_INTERNAL", false},
    {"ohos.permission.MANAGE_FINGERPRINT_AUTH", false},
    {"ohos.permission.ACCESS_PIN_AUTH",         false},
    {"ohos.permission.ACCESS_AUTH_RESPOOL",     false},
    {"ohos.permission.ENFORCE_USER_IDM",        false},
    {"ohos.permission.GET_RUNNING_INFO",        false},
    {"ohos.permission.CLEAN_APPLICATION_DATA",  false},
    {"ohos.permission.RUNNING_STATE_OBSERVER",  false},
    {"ohos.permission.CAPTURE_SCREEN",          false},
    {"ohos.permission.GET_WIFI_INFO",           false},
    {"ohos.permission.GET_WIFI_INFO_INTERNAL",  false},
    {"ohos.permission.SET_WIFI_INFO",           false},
    {"ohos.permission.GET_WIFI_PEERS_MAC",      false},
    {"ohos.permission.GET_WIFI_LOCAL_MAC",      false},
    {"ohos.permission.GET_WIFI_CONFIG",         false},
    {"ohos.permission.SET_WIFI_CONFIG",         false},
    {"ohos.permission.MANAGE_WIFI_CONNECTION",  false},
    {"ohos.permission.DUMP",                    false},
    {"ohos.permission.MANAGE_WIFI_HOTSPOT",     false},
    {"ohos.permission.GET_ALL_APP_ACCOUNTS",    false},
    {"ohos.permission.MANAGE_SECURE_SETTINGS",  false},
    {"ohos.permission.READ_DFX_SYSEVENT",       false},
    {"ohos.permission.READ_HIVIEW_SYSTEM",      false},
    {"ohos.permission.WRITE_HIVIEW_SYSTEM",     false},
    {"ohos.permission.SUBSCRIBE_SWING_ABILITY",     false},
    {"ohos.permission.MANAGER_SWING_MOTION",     false},
    {"ohos.permission.MANAGE_ENTERPRISE_DEVICE_ADMIN",      false},
    {"ohos.permission.SET_ENTERPRISE_INFO",                 false},
    {"ohos.permission.ACCESS_BUNDLE_DIR",                   false},
    {"ohos.permission.ENTERPRISE_SUBSCRIBE_MANAGED_EVENT",  false},
    {"ohos.permission.ENTERPRISE_SET_DATETIME",             false},
    {"ohos.permission.ENTERPRISE_GET_DEVICE_INFO",          false},
    {"ohos.permission.ENTERPRISE_RESET_DEVICE",             false},
    {"ohos.permission.ENTERPRISE_SET_WIFI",                 false},
    {"ohos.permission.ENTERPRISE_GET_NETWORK_INFO",         false},
    {"ohos.permission.ENTERPRISE_SET_ACCOUNT_POLICY",       false},
    {"ohos.permission.ENTERPRISE_SET_BUNDLE_INSTALL_POLICY", false},
    {"ohos.permission.ENTERPRISE_SET_NETWORK",              false},
    {"ohos.permission.ENTERPRISE_MANAGE_SET_APP_RUNNING_POLICY", false},
    {"ohos.permission.ENTERPRISE_SET_SCREENOFF_TIME",       false},
    {"ohos.permission.ENTERPRISE_MANAGE_SECURITY",          false},
    {"ohos.permission.ENTERPRISE_MANAGE_BLUETOOTH",         false},
    {"ohos.permission.ENTERPRISE_MANAGE_WIFI",              false},
    {"ohos.permission.ENTERPRISE_MANAGE_RESTRICTIONS",      false},
    {"ohos.permission.ENTERPRISE_MANAGE_APPLICATION",       false},
    {"ohos.permission.ENTERPRISE_MANAGE_LOCATION",          false},
    {"ohos.permission.ENTERPRISE_REBOOT",                   false},
    {"ohos.permission.ENTERPRISE_LOCK_DEVICE",              false},
    {"ohos.permission.ENTERPRISE_GET_SETTINGS",             false},
    {"ohos.permission.ENTERPRISE_MANAGE_SETTINGS",          false},
    {"ohos.permission.ENTERPRISE_INSTALL_BUNDLE",           false},
    {"ohos.permission.ENTERPRISE_MANAGE_CERTIFICATE",       false},
    {"ohos.permission.ENTERPRISE_MANAGE_SYSTEM",            false},
    {"ohos.permission.ENTERPRISE_RESTRICT_POLICY",          false},
    {"ohos.permission.ENTERPRISE_MANAGE_USB",               false},
    {"ohos.permission.ENTERPRISE_MANAGE_NETWORK",           false},
    {"ohos.permission.ENTERPRISE_SET_BROWSER_POLICY",       false},
    {"ohos.permission.ENTERPRISE_OPERATE_DEVICE",           false},
    {"ohos.permission.ENTERPRISE_ADMIN_MANAGE",             false},
    {"ohos.permission.ENTERPRISE_CONFIG",                   false},
    {"ohos.permission.NFC_TAG",                             false},
    {"ohos.permission.NFC_CARD_EMULATION",                  false},
    {"ohos.permission.PERMISSION_USED_STATS",               false},
    {"ohos.permission.NOTIFICATION_AGENT_CONTROLLER",       false},
    {"ohos.permission.MOUNT_UNMOUNT_MANAGER",               false},
    {"ohos.permission.MOUNT_FORMAT_MANAGER",                false},
    {"ohos.permission.STORAGE_MANAGER",                     false},
    {"ohos.permission.BACKUP",                              false},
    {"ohos.permission.CLOUDFILE_SYNC_MANAGER",              false},
    {"ohos.permission.CLOUDFILE_SYNC",                      false},
    {"ohos.permission.FILE_ACCESS_MANAGER",                 false},
    {"ohos.permission.GET_DEFAULT_APPLICATION",             false},
    {"ohos.permission.SET_DEFAULT_APPLICATION",             false},
    {"ohos.permission.ACCESS_IDS",                          false},
    {"ohos.permission.MANAGE_DISPOSED_APP_STATUS",          false},
    {"ohos.permission.GET_DISPOSED_APP_STATUS",          false},
    {"ohos.permission.ACCESS_DLP_FILE",                     false},
    {"ohos.permission.PROVISIONING_MESSAGE",                false},
    {"ohos.permission.ACCESS_SYSTEM_SETTINGS",              false},
    {"ohos.permission.ABILITY_BACKGROUND_COMMUNICATION",    false},
    {"ohos.permission.securityguard.REPORT_SECURITY_INFO",  false},
    {"ohos.permission.securityguard.REQUEST_SECURITY_MODEL_RESULT", false},
    {"ohos.permission.securityguard.REQUEST_SECURITY_EVENT_INFO",   false},
    {"ohos.permission.ACCESS_CERT_MANAGER_INTERNAL",        false},
    {"ohos.permission.ACCESS_CERT_MANAGER",                 false},
    {"ohos.permission.GET_LOCAL_ACCOUNTS",                  false},
    {"ohos.permission.MANAGE_DISTRIBUTED_ACCOUNTS",         false},
    {"ohos.permission.GET_DISTRIBUTED_ACCOUNTS",            false},
    {"ohos.permission.READ_ACCESSIBILITY_CONFIG",           false},
    {"ohos.permission.WRITE_ACCESSIBILITY_CONFIG",          false},
    {"ohos.permission.ACCESS_PUSH_SERVICE",                 false},
    {"ohos.permission.READ_APP_PUSH_DATA",                  false},
    {"ohos.permission.WRITE_APP_PUSH_DATA",                 false},
    {"ohos.permission.MANAGE_AUDIO_CONFIG",                 false},
    {"ohos.permission.MANAGE_CAMERA_CONFIG",                false},
    {"ohos.permission.CAMERA_CONTROL",                      false},
    {"ohos.permission.RECEIVER_STARTUP_COMPLETED",          false},
    {"ohos.permission.ACCESS_SERVICE_DM",                   false},
    {"ohos.permission.RUN_ANY_CODE",                        false},
    {"ohos.permission.PUBLISH_SYSTEM_COMMON_EVENT",         false},
    {"ohos.permission.ACCESS_SCREEN_LOCK_INNER",            false},
    {"ohos.permission.PRINT",                               false},
    {"ohos.permission.MANAGE_PRINT_JOB",                    false},
    {"ohos.permission.CHANGE_OVERLAY_ENABLED_STATE",        false},
    {"ohos.permission.CONNECT_CELLULAR_CALL_SERVICE",       false},
    {"ohos.permission.CONNECT_IMS_SERVICE",                 false},
    {"ohos.permission.ACCESS_SENSING_WITH_ULTRASOUND",      false},
    {"ohos.permission.PROXY_AUTHORIZATION_URI",             false},
    {"ohos.permission.INSTALL_ENTERPRISE_BUNDLE",           false},
    {"ohos.permission.ACCESS_CAST_ENGINE_MIRROR",           false},
    {"ohos.permission.ACCESS_CAST_ENGINE_STREAM",           false},
    {"ohos.permission.CLOUDDATA_CONFIG",                    false},
    {"ohos.permission.DEVICE_STANDBY_EXEMPTION",            false},
    {"ohos.permission.PERCEIVE_SMART_POWER_SCENARIO",       false},
    {"ohos.permission.RESTRICT_APPLICATION_ACTIVE",         false},
    {"ohos.permission.MANAGE_SENSOR",                       false},
    {"ohos.permission.UPLOAD_SESSION_MANAGER",              false},
    {"ohos.permission.PREPARE_APP_TERMINATE",               false},
    {"ohos.permission.MANAGE_ECOLOGICAL_RULE",              false},
    {"ohos.permission.GET_SCENE_CODE",                      false},
    {"ohos.permission.FILE_GUARD_MANAGER",                  false},
    {"ohos.permission.SET_FILE_GUARD_POLICY",               false},
    {"ohos.permission.securityguard.SET_MODEL_STATE",       false},
    {"ohos.permission.hsdr.HSDR_ACCESS",                    false},
    {"ohos.permission.SUPPORT_USER_AUTH",                   false},
    {"ohos.permission.CAPTURE_VOICE_DOWNLINK_AUDIO",        false},
    {"ohos.permission.MANAGE_INTELLIGENT_VOICE",            false},
    {"ohos.permission.INSTALL_ENTERPRISE_MDM_BUNDLE",       false},
    {"ohos.permission.INSTALL_ENTERPRISE_NORMAL_BUNDLE",    false},
    {"ohos.permission.INSTALL_SELF_BUNDLE",                 false},
    {"ohos.permission.OBSERVE_FORM_RUNNING",                false},
    {"ohos.permission.MANAGE_DEVICE_AUTH_CRED",             false},
    {"ohos.permission.UNINSTALL_BUNDLE",                    false},
    {"ohos.permission.RECOVER_BUNDLE",                      false},
    {"ohos.permission.GET_DOMAIN_ACCOUNTS",                 false},
    {"ohos.permission.SET_UNREMOVABLE_NOTIFICATION",        false},
    {"ohos.permission.QUERY_ACCESSIBILITY_ELEMENT",         false},
    {"ohos.permission.ACTIVATE_THEME_PACKAGE",              false},
    {"ohos.permission.ATTEST_KEY",                          false},
    {"ohos.permission.WAKEUP_VOICE",                        false},
    {"ohos.permission.WAKEUP_VISION",                       false},
    {"ohos.permission.ENABLE_DISTRIBUTED_HARDWARE",         false},
    {"ohos.permission.ACCESS_DISTRIBUTED_HARDWARE",         false},
    {"ohos.permission.INSTANTSHARE_SWITCH_CONTROL",         false},
    {"ohos.permission.ACCESS_INSTANTSHARE_SERVICE",         false},
    {"ohos.permission.ACCESS_INSTANTSHARE_PRIVATE_ABILITY", false},
    {"ohos.permission.SECURE_PASTE",                        false},
    {"ohos.permission.ACCESS_MCP_AUTHORIZATION",            false},
    {"ohos.permission.GET_BUNDLE_RESOURCES",                false},
    {"ohos.permission.SET_CODE_PROTECT_INFO",               false},
    {"ohos.permission.SET_ADVANCED_SECURITY_MODE",          false},
    {"ohos.permission.SET_DEVELOPER_MODE",                  false},
    {"ohos.permission.RUN_DYN_CODE",                        false},
    {"ohos.permission.COOPERATE_MANAGER",                   false},
    {"ohos.permission.PERCEIVE_TRAIL",                      false},
    {"ohos.permission.DISABLE_PERMISSION_DIALOG",           false},
    {"ohos.permission.EXECUTE_INSIGHT_INTENT",              false},
    {"ohos.permission.PRELOAD_UI_EXTENSION_ABILITY",        false},
    {"ohos.permission.MANAGE_ACTIVATION_LOCK",              false},
    {"ohos.permission.VERIFY_ACTIVATION_LOCK",              false},
    {"ohos.permission.MANAGE_PRIVATE_PHOTOS",               false},
    {"ohos.permission.ACCESS_OUC",                          false},
    {"ohos.permission.TRUSTED_RING_HASH_DATA_PERMISSION",   false},
    {"ohos.permission.QUERY_TRUSTED_RING_USER_INFO",        false},
    {"ohos.permission.MANAGE_TRUSTED_RING",                 false},
    {"ohos.permission.USE_TRUSTED_RING",                    false},
    {"ohos.permission.INPUT_CONTROL_DISPATCHING",           false},
    {"ohos.permission.INTERCEPT_INPUT_EVENT",               false},
    {"ohos.permission.LAUNCH_SPAMSHIELD_PAGE",              false},
    {"ohos.permission.ACCESS_SPAMSHIELD_SERVICE",           false},
    {"ohos.permission.ACCESS_SECURITY_PRIVACY_CENTER",      false},
    {"ohos.permission.GET_SECURITY_PRIVACY_ADVICE",         false},
    {"ohos.permission.SET_SECURITY_PRIVACY_ADVICE",         false},
    {"ohos.permission.USE_SECURITY_PRIVACY_MESSAGER",       false},
    {"ohos.permission.GET_PRIVACY_INDICATOR",               false},
    {"ohos.permission.SET_PRIVACY_INDICATOR",               false},
    {"ohos.permission.EXEMPT_PRIVACY_INDICATOR",            false},
    {"ohos.permission.EXEMPT_CAMERA_PRIVACY_INDICATOR",     false},
    {"ohos.permission.EXEMPT_MICROPHONE_PRIVACY_INDICATOR", false},
    {"ohos.permission.EXEMPT_LOCATION_PRIVACY_INDICATOR",   false},
    {"ohos.permission.GET_SUPER_PRIVACY",                   false},
    {"ohos.permission.SET_SUPER_PRIVACY",                   false},
    {"ohos.permission.RECORD_VOICE_CALL",                   false},
    {"ohos.permission.MANAGE_APP_INSTALL_INFO",             false},
    {"ohos.permission.RECEIVE_APP_INSTALL_INFO_CHANGE",     false},
    {"ohos.permission.ACCESS_ADVANCED_SECURITY_MODE",       false},
    {"ohos.permission.STORE_PERSISTENT_DATA",               false},
    {"ohos.permission.ACCESS_HIVIEWX",                      false},
    {"ohos.permission.ACCESS_PASSWORDVAULT_ABILITY",        false},
    {"ohos.permission.ACCESS_LOWPOWER_MANAGER",             false},
    {"ohos.permission.ACCESS_DDK_USB",                      false},
    {"ohos.permission.ACCESS_EXTENSIONAL_DEVICE_DRIVER",    false},
    {"ohos.permission.ACCESS_TEXTAUTOFILL_ABILITY",         false},
    {"ohos.permission.ACCESS_DDK_HID",                      false},
    {"ohos.permission.MANAGE_APP_BOOT",                     false},
    {"ohos.permission.ACCESS_HIVIEWCARE",                   false},
    {"ohos.permission.CONNECT_UI_EXTENSION_ABILITY",        false},
    {"ohos.permission.FILE_ACCESS_PERSIST",                 false},
    {"ohos.permission.SET_SANDBOX_POLICY",                  false},
    {"ohos.permission.ACCESS_ACCOUNT_KIT_SERVICE",          false},
    {"ohos.permission.REQUEST_ANONYMOUS_ATTEST",            false},
    {"ohos.permission.ACCESS_ACCOUNT_KIT_UI",               false},
    {"ohos.permission.READ_ACCOUNT_LOGIN_STATE",            false},
    {"ohos.permission.WRITE_ACCOUNT_LOGIN_STATE",           false},
    {"ohos.permission.START_ABILITY_WITH_ANIMATION",        false},
    {"ohos.permission.START_RECENT_ABILITY",                false},
    {"ohos.permission.READ_CLOUD_SYNC_CONFIG",              false},
    {"ohos.permission.MANAGE_CLOUD_SYNC_CONFIG",            false},
    {"ohos.permission.ACCESS_FINDDEVICE",                   false},
    {"ohos.permission.MANAGE_FINDSERVICE",                  false},
    {"ohos.permission.TRIGGER_ACTIVATIONLOCK",              false},
    {"ohos.permission.MANAGE_USB_CONFIG",                   false},
    {"ohos.permission.WRITE_PRIVACY_PUSH_DATA",             false},
    {"ohos.permission.READ_PRIVACY_PUSH_DATA",              false},
    {"ohos.permission.MANAGE_HAP_TOKENID",                  false},
    {"ohos.permission.REPORT_RESOURCE_SCHEDULE_EVENT",      false},
    {"ohos.permission.SEND_TASK_COMPLETE_EVENT",            false},
    {"ohos.permission.GET_SUSPEND_STATE",                   false},
    {"ohos.permission.MANAGE_APP_BOOT_INTERNAL",            false},
    {"ohos.permission.REGISTER_APP_DEBUG_LISTENER",         false},
    {"ohos.permission.ATTACH_APP_DEBUG",                    false},
    {"ohos.permission.NOTIFY_DEBUG_ASSERT_RESULT",          false},
    {"ohos.permission.CHANGE_DISPLAYMODE",                  false},
    {"ohos.permission.ACCESS_MEDIALIB_THUMB_DB",            false},
    {"ohos.permission.MIGRATE_DATA",                        false},
    {"ohos.permission.ACCESS_DYNAMIC_ICON",                 false},
    {"ohos.permission.CHANGE_BUNDLE_UNINSTALL_STATE",       false},
    {"ohos.permission.MONITOR_DEVICE_NETWORK_STATE",        false},
    {"ohos.permission.SYNC_PROFILE_DP",                     false},
    {"ohos.permission.ACCESS_SERVICE_DP",                   false},
    {"ohos.permission.ACCESS_PROTOCOL_DFX_STATE",           false},
    {"ohos.permission.ACCESS_SERVICE_NAVIGATION_INFO",      false},
    {"ohos.permission.MANAGE_STYLUS_EVENT",                 false},
    {"ohos.permission.WRITE_GTOKEN_POLICY",                 false},
    {"ohos.permission.READ_GTOKEN_POLICY",                  false},
    {"ohos.permission.ENABLE_PROFILER",                     false},
    {"ohos.permission.PRELOAD_APPLICATION",                 false},
    {"ohos.permission.USE_CLOUD_DRIVE_SERVICE",             false},
    {"ohos.permission.USE_CLOUD_BACKUP_SERVICE",            false},
    {"ohos.permission.USE_CLOUD_COMMON_SERVICE",            false},
    {"ohos.permission.START_DLP_CRED",                      false},
    {"ohos.permission.START_SHORTCUT",                      false},
    {"ohos.permission.MANAGE_INPUT_INFRARED_EMITTER",       false},
    {"ohos.permission.SET_PROCESS_CACHE_STATE",             false},
    {"ohos.permission.ACCESS_PRIVATE_SPACE_MANAGER",        false},
    {"ohos.permission.ACCESS_PRIVATE_SPACE_PASSWORD_PROTECT", false},
    {"ohos.permission.ACCESS_LOCAL_BACKUP", false},
    {"ohos.permission.ACCESS_SYSTEM_APP_CERT",              false},
    {"ohos.permission.ACCESS_USER_TRUSTED_CERT",            false},
    {"ohos.permission.CAST_AUDIO_OUTPUT",                   false},
    {"ohos.permission.GRANT_URI_PERMISSION_PRIVILEGED",     false},
    {"ohos.permission.UPDATE_APP_CONFIGURATION",            false},
    {"ohos.permission.KILL_APP_PROCESSES",                  false},
    {"ohos.permission.ACCESS_SCREEN_LOCK_MEDIA_DATA",       false},
    {"ohos.permission.ACCESS_SCREEN_LOCK_ALL_DATA",         false},
    {"ohos.permission.MANAGE_SYSTEM_ABILITY",               false},
    {"ohos.permission.ACCESS_EXT_SYSTEM_ABILITY",           false},
    {"ohos.permission.WRITE_RINGTONE",                      false},
    {"ohos.permission.UPDATE_QUICKFIX",                     false},
    {"ohos.permission.GET_QUICKFIX_INFO",                   false},
    {"ohos.permission.CHECK_QUICKFIX_RESULT",               false},
    {"ohos.permission.GET_ACCOUNT_MINORS_INFO",             false},
    {"ohos.permission.INTERACT_ACROSS_LOCAL_ACCOUNTS_AS_USER", false},
    {"ohos.permission.ACCESS_LOCAL_THEME",                  false},
    {"ohos.permission.ACCESS_SHADER_CACHE_DIR",             false},
    {"ohos.permission.INSTALL_CLONE_BUNDLE",                false},
    {"ohos.permission.UNINSTALL_CLONE_BUNDLE",              false},
    {"ohos.permission.PROTECT_SCREEN_LOCK_DATA",            false},
    {"ohos.permission.MANAGE_SETTINGS",                     false},
    {"ohos.permission.ACCESS_DEVICE_COLLABORATION_PRIVATE_ABILITY",                     false},
    {"ohos.permission.ACCESS_RINGTONE_RESOURCE",            false},
    {"ohos.permission.ACCESS_FILE_CONTENT_SHARE",           false},
    {"ohos.permission.ACCESS_SEARCH_SERVICE",               false},
    {"ohos.permission.ACCESS_SCREEN_LOCK",                  false},
    {"ohos.permission.MANAGE_SOFTBUS_NETWORK",              false},
    {"ohos.permission.MANAGE_FINDNETWORK",                  false},
    {"ohos.permission.SET_FOREGROUND_HAP_REMINDER",         false},
    {"ohos.permission.OPERATE_FINDNETWORK",                 false},
    {"ohos.permission.QUERY_FINDNETWORK_LOCATION",          false},
    {"ohos.permission.INJECT_INPUT_EVENT",              false},
    {"ohos.permission.ACCESS_SUBSCRIPTION_CAPSULE_DATA",              false},
    {"ohos.permission.PRE_START_ATOMIC_SERVICE",            false},
    {"ohos.permission.UPDATE_CALENDAR_RRULE",            false},
    {"ohos.permission.QUERY_SECURITY_EVENT",                        false},
    {"ohos.permission.REPORT_SECURITY_EVENT",                       false},
    {"ohos.permission.QUERY_SECURITY_MODEL_RESULT",                 false},
    {"ohos.permission.MANAGE_SECURITY_GUARD_CONFIG",                false},
    {"ohos.permission.COLLECT_SECURITY_EVENT",                      false},
    {"ohos.permission.QUERY_AUDIT_EVENT",                           false},
    {"ohos.permission.QUERY_SECURITY_POLICY_FROM_CLOUD",            false},
    {"ohos.permission.REPORT_SECURITY_EVENT_TO_CLOUD",              false},
    {"ohos.permission.CONNECT_FORM_EXTENSION",              false},
    {"ohos.permission.CONNECT_WORK_SCHEDULER_EXTENSION",    false},
    {"ohos.permission.CONNECT_INPUT_METHOD_EXTENSION",      false},
    {"ohos.permission.CONNECT_ACCESSIBILITY_EXTENSION",     false},
    {"ohos.permission.CONNECT_STATIC_SUBSCRIBER_EXTENSION", false},
    {"ohos.permission.CONNECT_WALLPAPER_EXTENSION",         false},
    {"ohos.permission.CONNECT_BACKUP_EXTENSION",            false},
    {"ohos.permission.CONNECT_ENTERPRISE_ADMIN_EXTENSION",  false},
    {"ohos.permission.CONNECT_FILE_ACCESS_EXTENSION",       false},
    {"ohos.permission.CONNECT_PRINT_EXTENSION",             false},
    {"ohos.permission.CONNECT_DRIVER_EXTENSION",            false},
    {"ohos.permission.CONNECT_APP_ACCOUNT_AUTHORIZATION_EXTENSION",    false},
    {"ohos.permission.CONNECT_REMOTE_NOTIFICATION_EXTENSION",          false},
    {"ohos.permission.CONNECT_REMOTE_LOCATION_EXTENSION",   false},
    {"ohos.permission.CONNECT_VPN_EXTENSION",               false},
    {"ohos.permission.KILL_PROCESS_DEPENDED_ON_ARKWEB",     false},
    {"ohos.permission.CONTROL_LOCATION_SWITCH",               false},
    {"ohos.permission.MOCK_LOCATION",     false},
    {"ohos.permission.ALLOW_TIPS_ACCESS",   false},
    {"ohos.permission.ACCESS_SCAN_SERVICE",                 false},
    {"ohos.permission.ACCESS_FACTORY_OTA_DIR",   false},
    {"ohos.permission.MICROPHONE_CONTROL",   false},
    {"ohos.permission.MANAGE_MOUSE_CURSOR",                 false},
    {"ohos.permission.FILTER_INPUT_EVENT",                  false},
    {"ohos.permission.INPUT_PANEL_STATUS_PUBLISHER",        false},
    {"ohos.permission.PUBLISH_LOCATION_EVENT",              false},
    {"ohos.permission.ACCESS_MULTICORE_HYBRID_ABILITY",     false},
};

bool TransferPermissionToOpcode(const std::string& permission, uint32_t& opCode)
{
    size_t size = g_permMap.size();
    for (size_t i = 0; i < size; i++) {
        std::pair it = g_permMap[i];
        if (permission == it.first) {
            opCode = i;
            return true;
        }
    }
    return false;
}

bool TransferOpcodeToPermission(uint32_t opCode, std::string& permission)
{
    if (opCode >= MAX_PERM_SIZE || opCode >= g_permMap.size()) {
        return false;
    }
    permission = g_permMap[opCode].first;
    return true;
}
} // namespace AccessToken
} // namespace Security
} // namespace OHOS