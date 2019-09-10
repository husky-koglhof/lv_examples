#ifndef _LV_EX_SETTINGS_3_H
#define _LV_EX_SETTINGS_3_H

#define _DUALRATE    0
#define _DR_TH       1
#define _DR_ST       2
#define _SYSTEM      16
#define _WIFI        24
#define _DISPLAY     27
#define _INFO        30

#define _WIFI_AP     100
#define _WIFI_SSID   101
#define _WIFI_PASS   102
#define _WIFI_AP_SSID       107
#define _WIFI_AP_PASS       108
#define _INFO_WIFI          109
#define _WIFI_BUTTON        110

#define _IPADDR             111
#define _GATEWAY            112
#define _NETMASK            113
#define _DNS                114
#define _CLIENTNAME         115

#define _USE_SLIDER			230

#define _ROOT        254
#define _LAST        255

void lv_ex_settings_3(void);
#endif
