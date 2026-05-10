#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>

namespace android {

// Maps Android numeric UID/GID to their symbolic names.
// Mirrors the Python dict from the original tar_repacker.py.
inline const std::unordered_map<uint32_t, std::string_view> kIdToName = {
    {0, "root"},       {1, "daemon"},     {2, "bin"},        {3, "sys"},
    {1000, "system"},  {1001, "radio"},   {1002, "bluetooth"},{1003, "graphics"},
    {1004, "input"},   {1005, "audio"},   {1006, "camera"},  {1007, "log"},
    {1008, "compass"}, {1009, "mount"},   {1010, "wifi"},    {1011, "adb"},
    {1012, "install"}, {1013, "media"},   {1014, "dhcp"},    {1015, "sdcard_rw"},
    {1016, "vpn"},     {1017, "keystore"},{1018, "usb"},     {1019, "drm"},
    {1020, "mdnsr"},   {1021, "gps"},     {1022, "unused1"}, {1023, "media_rw"},
    {1024, "mtp"},     {1025, "unused2"}, {1026, "drmrpc"},  {1027, "nfc"},
    {1028, "sdcard_r"},{1029, "clat"},    {1030, "loop_radio"},{1031, "media_drm"},
    {1032, "package_info"}, {1033, "sdcard_pics"}, {1034, "sdcard_av"},
    {1035, "sdcard_all"},   {1036, "logd"},        {1037, "shared_relro"},
    {1038, "dbus"},    {1039, "tlsdate"}, {1040, "media_ex"},{1041, "audioserver"},
    {1042, "metrics_coll"}, {1043, "metricsd"},    {1044, "webserv"},
    {1045, "debuggerd"},{1046, "media_codec"},{1047, "cameraserver"},
    {1048, "firewall"},{1049, "trunks"},  {1050, "nvram"},   {1051, "dns"},
    {1052, "dns_tether"},{1053, "webview_zygote"},{1054, "vehicle_network"},
    {1055, "media_audio"},{1056, "media_video"},{1057, "media_image"},
    {1058, "tombstoned"},{1059, "media_obb"},{1060, "ese"},  {1061, "ota_update"},
    {1062, "automotive_evs"},{1063, "lowpan"},{1064, "hsm"},{1065, "reserved_disk"},
    {1066, "statsd"},  {1067, "incidentd"},{1068, "secure_element"},{1069, "lmkd"},
    {1070, "llkd"},    {1071, "iorapd"},  {1072, "gpu_service"},{1073, "network_stack"},
    {1074, "gsid"},    {1075, "fsverity_cert"},{1076, "credstore"},
    {1077, "external_storage"},{1078, "ext_data_rw"},{1079, "ext_obb_rw"},
    {1080, "context_hub"},{1081, "virtualizationservice"},{1082, "artd"},
    {1083, "uwb"},     {1084, "thread_network"},{1085, "diced"},{1086, "dmesgd"},
    {1087, "jc_weaver"},{1088, "jc_strongbox"},{1089, "jc_identitycred"},
    {1090, "sdk_sandbox"},{1091, "security_log_writer"},{1092, "prng_seeder"},
    {1093, "uprobestats"},{1094, "cros_ec"},
    {1300, "thememan"},{1301, "audit"},
    {2000, "shell"},   {2001, "cache"},   {2002, "diag"},
    {2900, "oem_reserved_start"},{2950, "qcom_diag"},{2951, "rfs"},
    {2952, "rfs_shared"},{2999, "oem_reserved_end"},
    {3001, "net_bt_admin"},{3002, "net_bt"},{3003, "inet"},{3004, "net_raw"},
    {3005, "net_admin"},{3006, "net_bw_stats"},{3007, "net_bw_acct"},
    {3008, "net_bt_stack"},{3009, "readproc"},{3010, "wakelock"},{3011, "uhid"},
    {3012, "readtracefs"},{3013, "virtualmachine"},{3014, "rfs_shared_old"},
    {5000, "oem_reserved_2_start"},{5999, "oem_reserved_2_end"},
    {6000, "system_reserved_start"},{6499, "system_reserved_end"},
    {6500, "odm_reserved_start"},{6999, "odm_reserved_end"},
    {7000, "product_reserved_start"},{7499, "product_reserved_end"},
    {7500, "system_ext_reserved_start"},{7999, "system_ext_reserved_end"},
    {9000, "mot_accy"},{9001, "mot_pwric"},{9002, "mot_usb"},{9003, "mot_drm"},
    {9004, "mot_tcmd"},{9005, "mot_sec_rtc"},{9006, "mot_tombstone"},
    {9007, "mot_tpapi"},{9008, "mot_secclkd"},{9009, "mot_whisper"},
    {9010, "mot_caif"},{9011, "mot_dlna"},
    {9997, "everybody"},{9998, "misc"},{9999, "nobody"},
    {10000, "app"},   {19999, "app_end"},
    {20000, "cache_gid_start"},{29999, "cache_gid_end"},
    {30000, "ext_gid_start"},  {39999, "ext_gid_end"},
    {40000, "ext_cache_gid_start"},{49999, "ext_cache_gid_end"},
    {50000, "shared_gid_start"},{59999, "shared_gid_end"},
    {65534, "overflowuid"},
    {90000, "isolated_start"},{99999, "isolated_end"},
    {100000, "user"},
};

// Returns the symbolic name for an Android UID/GID, or the decimal string if unknown.
inline std::string id_to_name(uint32_t id) {
    auto it = kIdToName.find(id);
    if (it != kIdToName.end())
        return std::string(it->second);
    return std::to_string(id);
}

} // namespace android
