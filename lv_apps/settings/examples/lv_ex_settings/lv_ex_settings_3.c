/**
 * @file lv_ex_settings_3.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "../../src/lv_settings/lv_settings.h"
#include "lv_drivers/indev/keyboard.h"
#include "lv_drivers/indev/mousewheel.h"
#include "lv_ex_settings_3.h"

#define USE_SIMULATOR
#ifdef USE_SIMULATOR
#include "../../src/lv_wifi/lv_wifi.h"
#else
#include "WiFi.h"
#endif

/*********************
 *      DEFINES
 *********************/
#ifndef LV_SETTINGS_KEYBOARD
#define LV_SETTINGS_KEYBOARD 0
#endif

#ifndef LV_SETTINGS_MOUSEWHEEL
#define LV_SETTINGS_MOUSEWHEEL 0
#endif

/**********************
*      TYPEDEFS
**********************/

/**********************
*  STATIC PROTOTYPES
**********************/
static void root_event_cb(lv_obj_t * btn, lv_event_t e);
static void main_menu_event_cb(lv_obj_t * btn, lv_event_t e);
static void wifi_event_cb(lv_obj_t * btn, lv_event_t e);
static void info_menu_event_cb(lv_obj_t * btn, lv_event_t e);

/**********************
*  STATIC VARIABLES
**********************/
bool _wifiEnabled = false;
char WIFI_SSID_DEFAULT[23] = "Select...\nSearch...\n";
char WIFI_AP_DEFAULT[20] = "arcs";
char WIFI_AP_PASS_DEFAULT[20] = "test1234";

/*Declare items*/
static lv_settings_item_t root_item = {
  .type = LV_SETTINGS_TYPE_ROOT,
  .id = _ROOT,
  .name = "Settings",
  .value = "",
  .hidden = false,
};

static lv_settings_item_t main_menu_items[] = {
  {.type = LV_SETTINGS_TYPE_LIST_BTN, .id = _SYSTEM, .name="System", .value="WiFi", .hidden = false},
  {.type = LV_SETTINGS_TYPE_LIST_BTN, .id = _INFO, .name="Info", .value="WiFi", .hidden = false},
  {.type = LV_SETTINGS_TYPE_INV, .id = _LAST},     //Mark the last item
};

/* System has submenus */
static lv_settings_item_t system_menu_items[] = {
  {.type = LV_SETTINGS_TYPE_LIST_BTN, .id = _WIFI, .name="Wifi", .value="Wifi AP, Client", .hidden = false},
  {.type = LV_SETTINGS_TYPE_INV, .id = _LAST},     //Mark the last item
};

typedef enum {
    WIFI_STATE = 0,
    WIFI_USE_AP,
    WIFI_SSID,
    WIFI_PASS,
    WIFI_CONNECT_BUTTON,
}wifi_item_t;

static lv_settings_item_t wifi_items[] = {
  {.type = LV_SETTINGS_TYPE_SW, .id = _WIFI, .name = "WiFi State", .value = "Disabled", .hidden = false},
  {.type = LV_SETTINGS_TYPE_SW, .id = _WIFI_AP, .name = "Use AP", .value = "Disabled", .hidden = true},
  {.type = LV_SETTINGS_TYPE_DDLIST, .id = _WIFI_SSID, .name = "SSID", .value = WIFI_SSID_DEFAULT, .hidden = true},
  {.type = LV_SETTINGS_TYPE_PASS, .id = _WIFI_PASS, .name = "Password", .value = WIFI_AP_PASS_DEFAULT, .hidden = true},
  {.type = LV_SETTINGS_TYPE_BTN, .id = _WIFI_BUTTON, .name = "Connect", .value = "Connect", .hidden = true},
  {.type = LV_SETTINGS_TYPE_INV, .id = _LAST},     //Mark the last item
};

static lv_settings_item_t info_menu_items[] = {
  {.type = LV_SETTINGS_TYPE_LIST_BTN, .id = _INFO_WIFI, .name="Wifi", .value="SSID, IP-Address, etc.", .hidden = false},
  {.type = LV_SETTINGS_TYPE_INV, .id = _LAST},     //Mark the last item
};

typedef enum {
    SSID = 0,
    IP_ADDR,
    GATEWAY,
    NETMASK,
    DNS,
    CLIENT_NAME,
}info_wifi_item_t;

static lv_settings_item_t info_wifi_items[] = {
  {.type = LV_SETTINGS_TYPE_DESCR, .id = _WIFI_SSID, .name = "WiFi", .value = "", .hidden = false},
  {.type = LV_SETTINGS_TYPE_DESCR, .id = _IPADDR, .name = "IP-Address", .value = "", .hidden = false},
  {.type = LV_SETTINGS_TYPE_DESCR, .id = _GATEWAY, .name = "Gateway", .value = "", .hidden = false},
  {.type = LV_SETTINGS_TYPE_DESCR, .id = _NETMASK, .name = "Netmask", .value = "", .hidden = false},
  {.type = LV_SETTINGS_TYPE_DESCR, .id = _DNS, .name = "DNS", .value = "", .hidden = false},
  {.type = LV_SETTINGS_TYPE_DESCR, .id = _CLIENTNAME, .name = "Name", .value = "", .hidden = false},
  {.type = LV_SETTINGS_TYPE_INV, .id = _LAST},     //Mark the last item
};

/**********************
*      MACROS
**********************/

/**********************
*   GLOBAL FUNCTIONS
**********************/
void write_config();

/**********************
*   STATIC FUNCTIONS
**********************/


/* Helper function */
lv_settings_item_t * ssidList;
lv_settings_item_t * wifiConnectState;

typedef struct {
    char ssid[20];
    char pass[20];
    bool secure;
    bool valid;
}lv_ssid_item_t;

#define SSID_LIST_SIZE 20
static lv_ssid_item_t ssid_items[SSID_LIST_SIZE];
static lv_ssid_item_t config_ssid_items[SSID_LIST_SIZE];

char * get_password_from_config(char ssid[20]) {
  lv_ssid_item_t * ssid_item;
  for (int32_t i = 0; i < SSID_LIST_SIZE; i++) {
    // ssid_item = &ssid_items[i];
    ssid_item = &config_ssid_items[i];

    if (ssid_item->valid == true && strcmp(ssid_item->ssid, ssid) == 0) {
      printf("got from config: ssid = %s, pass = %s, secure = %s\n", ssid_item->ssid, ssid_item->pass, ssid_item->secure ? "Yes": "No");
      // TODO: We must publish the password to the keyboard or something else...
      // At this moment we must open the keyboard and press "enter"
      return ssid_item->pass;
    }
  }

  return "";
}

static void wifiHelper(lv_task_t * task) {
#ifdef USE_SIMULATOR
  int count = scanComplete();
#else
  int count = WiFi.scanComplete();
#endif
  if (count >= 0) {
	LV_LOG_WARN("scan done");
    char wifiString[200];
    if (count == 0) {
    	LV_LOG_WARN("no networks found");
    } else {
      // LV_LOG_WARN(count);
      LV_LOG_WARN(" networks found");
      sprintf(wifiString, "%s", WIFI_SSID_DEFAULT);
      /* Clean out all ssid_items from list */
      for (int32_t i = 0; i < SSID_LIST_SIZE; i++) {
        lv_ssid_item_t * ssid_item = &ssid_items[i];
        ssid_item->valid = false;
        strcpy(ssid_item->pass, "");
      }

      for (int32_t i = 0; i < count; ++i) {
        // Print SSID and RSSI for each network found
#ifdef USE_SIMULATOR
        sprintf(wifiString, "%s%s (%d)%s\n", wifiString, getSSID(i), getRSSI(i), (getEncryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
#else
        sprintf(wifiString, "%s%s (%d)%s\n", wifiString, (WiFi.SSID(i)).c_str(), WiFi.RSSI(i), (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
#endif

        lv_ssid_item_t * ssid_item = &ssid_items[i];
#ifdef USE_SIMULATOR
        strcpy(ssid_item->ssid, getSSID(i));
#else
        strcpy(ssid_item->ssid, const_cast<char*>((WiFi.SSID(i)).c_str()));
#endif

        // Check if we know already a password, saved in config
        strcpy(ssid_item->pass, get_password_from_config(ssid_item->ssid));

#ifdef USE_SIMULATOR
        ssid_item->secure = (getEncryptionType(i) == WIFI_AUTH_OPEN) ? false : true;
#else
        ssid_item->secure = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? false : true;
#endif
        ssid_item->valid = true;
      }
      lv_settings_item_t * item;
      item = &wifi_items[WIFI_SSID]; // _WIFI_SSID

      item->value = wifiString;
      item->state = 0;
      // lv_settings_item_t * user_data = (lv_settings_item_t*)task->user_data;
      lv_settings_refr(ssidList);

      for (int32_t i = 0; i < SSID_LIST_SIZE; i++) {
        lv_ssid_item_t * ssid_item = &ssid_items[i];
        if (ssid_item->valid == true) {
          printf("ssid = %s, pass = %s, secure = %s\n", ssid_item->ssid, ssid_item->pass, ssid_item->secure ? "Yes": "No");
        }
      }
    }

    lv_task_del(task);
    lv_hide_preloader(500);
  }
}

static int32_t wifiCount = 0;
static void wifiConnect(lv_task_t * task) {
  wifiCount++;
  bool cancelScan = false;
  char * text = "Failed";
  if (wifiCount == 200) { // 100ms delay * 200 = 20000 = 20sec
    wifiCount = 0;
    cancelScan = true;
    text = "Delay";
#ifdef USE_SIMULATOR
  } else if (getWiFiStatus() == WL_CONNECT_FAILED) {
    cancelScan = true;
  } else {
    if (getWiFiStatus() == WL_CONNECTED) {
#else
  } else if (WiFi.status() == WL_CONNECT_FAILED) {
    cancelScan = true;
  } else {
    if (WiFi.status() == WL_CONNECTED) {
#endif
      cancelScan = true;
      text = "Connected";

      char * ssid = (char *)task->user_data;

      // Get Password from keyboard
      char * pass = lv_get_text_from_keyboard();
      printf("---> Password from keyboard = '%s'\n", pass);
      // If nothing is entered, get password from config
      if (strcmp(pass, "") == 0) {
        pass = get_password_from_config(ssid);
      }

      printf("searching for %s in ssid_items\n", ssid);
      for (int32_t i = 0; i < SSID_LIST_SIZE; i++) { // sizeof ssid_items
        lv_ssid_item_t * ssid_item = &ssid_items[i];
        printf("ssid = %s, pass = %s, secure = %s, valid = %s\n", ssid_item->ssid, ssid_item->pass, ssid_item->secure ? "Yes": "No", ssid_item->valid ? "Yes": "No");

        if (ssid_item->valid == true && strcmp(ssid, ssid_item->ssid) == 0) {
          // ssid_item->pass = pass;
          strcpy(ssid_item->pass, pass);
          printf("Found valid wifi_item: ssid = %s, pass = %s\n", ssid, pass);

          // TODO: Save back valid item to config on filesystem
          write_config();
        }
      }
/*
      for (byte i = 0; i < SSID_LIST_SIZE; i++) {
        lv_ssid_item_t * ssid_item = &ssid_items[i];
        if (ssid_item->valid == true) {
          printf("ssid = %s, pass = %s, secure = %s\n", ssid_item->ssid, ssid_item->pass, ssid_item->secure ? "Yes": "No");
        }
      }
*/
    }
  }

  if (cancelScan) {
    lv_task_del(task);
    lv_hide_preloader(500);

    wifiConnectState->name = text;
    lv_settings_refr(wifiConnectState);
  }
}

  /* */
/**********************
 *   STATIC FUNCTIONS
 **********************/

static void getSettings() {
  printf("------> getSettings called\n");
  lv_settings_item_t * item;

  item = &wifi_items[WIFI_STATE];
  item->state = _wifiEnabled;
  if (_wifiEnabled) {
    item->value = "Active";

    for(int8_t y = 1; y<5; y++) {
      item = &wifi_items[y];
      item->hidden = false;
    }
  } else {
    item->value = "Deactivated";

    for(int8_t y = 1; y<5; y++) {
      item = &wifi_items[y];
      item->hidden = true;
    }
  }
}

static char * _ssid;

static void useAPMode(bool mode) {
  if (mode == true) {
    lv_settings_item_t * item;

    item = &wifi_items[WIFI_USE_AP];
    item->value = "Enabled";
    lv_settings_refr(item);

    // Hide ddlist in AP Mode
    item = &wifi_items[WIFI_SSID];
    item->hidden = true;
    lv_settings_refr(item);

    // Change type to text
    item->type = LV_SETTINGS_TYPE_TEXT;
    item->value = WIFI_AP_DEFAULT;
    _ssid = WIFI_AP_DEFAULT;
    item->hidden = false;
    lv_settings_add(item);

    // Hide Password in AP Mode
    item = &wifi_items[WIFI_PASS];
    item->hidden = true;
    lv_settings_refr(item);

    // Change type to text
    item->type = LV_SETTINGS_TYPE_TEXT;
    item->value = WIFI_AP_PASS_DEFAULT;
    item->hidden = false;
    lv_settings_add(item);

    item = &wifi_items[WIFI_CONNECT_BUTTON];
    item->name = "Deactivated";
    item->value = "Activate";
    item->hidden = false;
    lv_settings_add(item);

    lv_hide_keyboard();
  } else {
    lv_settings_item_t * item;

    item = &wifi_items[WIFI_USE_AP];
    item->value = "Disabled";
    lv_settings_refr(item);

    // Show ddlist in Client Mode
    item = &wifi_items[WIFI_SSID];
    item->hidden = true;
    lv_settings_refr(item);

    // Change type to text
    item->type = LV_SETTINGS_TYPE_DDLIST;
    item->value = WIFI_SSID_DEFAULT;
    item->hidden = false;
    item->state = 0;
    lv_settings_add(item);

    // Show Password in Client Mode
    item = &wifi_items[WIFI_PASS];
    item->hidden = true;
    lv_settings_refr(item);

    // Change type to text
    item->type = LV_SETTINGS_TYPE_PASS;
    item->value = WIFI_AP_PASS_DEFAULT;
    item->hidden = true;
    lv_settings_add(item);

    lv_hide_keyboard();
  }
}

static void root_event_cb(lv_obj_t * btn, lv_event_t e) {
    (void)btn;  //Unused

    getSettings();
    //If the root element was clicked or asks for create a main menu
    if(e == LV_EVENT_CLICKED) {
        //Get the caller item
        lv_settings_item_t * act_item = (lv_settings_item_t *)lv_event_get_data();

        lv_settings_open_page(act_item, main_menu_event_cb);
    }
}

static void main_menu_event_cb(lv_obj_t * btn, lv_event_t e) {
  (void)btn;  //Unused

  //Get the caller item/
  lv_settings_item_t * act_item = (lv_settings_item_t *)lv_event_get_data();
  printf("main_menu_event_cb: event = %s, id = %d\n", e == LV_EVENT_REFRESH ? "Refresh" : "Clicked", act_item->id);
  if(e == LV_EVENT_REFRESH) {
    uint32_t i;
    if (act_item->id == _ROOT) {
      for(i = 0; main_menu_items[i].type != LV_SETTINGS_TYPE_INV; i++) {
        lv_settings_add(&main_menu_items[i]);
      }
    } else if (act_item->id == _SYSTEM) {
      for(i = 0; system_menu_items[i].type != LV_SETTINGS_TYPE_INV; i++) {
        lv_settings_add(&system_menu_items[i]);
      }
    } else if (act_item->id == _INFO) {
      for(i = 0; info_menu_items[i].type != LV_SETTINGS_TYPE_INV; i++) {
        lv_settings_add(&info_menu_items[i]);
      }
    }
  } else if(e == LV_EVENT_CLICKED) {
    if (act_item->id == _SYSTEM) {
	  lv_settings_open_page(act_item, main_menu_event_cb);
    } else if (act_item->id == _WIFI) {
      lv_settings_open_page(act_item, wifi_event_cb);
    } else if (act_item->id == _INFO) {
      lv_settings_open_page(act_item, main_menu_event_cb);
    } else if (act_item->id == _INFO_WIFI) {
      lv_settings_open_page(act_item, info_menu_event_cb);
    }
  }
}

/* Helper function */
char* subStr (char* input_string, char *separator, int segment_number) {
  char *act, *sub, *ptr;
  static char copy[100];
  int i;

  strcpy(copy, input_string);
  for (i = 1, act = copy; i <= segment_number; i++, act = NULL) {
    sub = strtok_r(act, separator, &ptr);
    if (sub == NULL) break;
  }
  return sub;
}
/* Helper function */

static void info_menu_event_cb(lv_obj_t * btn, lv_event_t e) {
  (void)btn;  //Unused

  lv_settings_item_t * act_item = (lv_settings_item_t *)lv_event_get_data();
  if(e == LV_EVENT_REFRESH) {
    printf("info_menu_event_cb: event = Refresh, id = %d, state = %d, value = %s, name = %s\n", act_item->id, act_item->state, act_item->value, act_item->name);
    if (act_item->id == _INFO_WIFI) {
      lv_settings_item_t * item;

      item = &info_wifi_items[SSID];
#ifdef USE_SIMULATOR
      item->value = getWiFiStatusText();
#else
      item->value = const_cast<char*>(WiFi.SSID().c_str());
#endif
      lv_settings_add(item);

      item = &info_wifi_items[IP_ADDR];
#ifdef USE_SIMULATOR
      item->value = getLocalIP();
#else
      item->value = const_cast<char*>(WiFi.localIP().toString().c_str());
#endif
      lv_settings_add(item);

      item = &info_wifi_items[GATEWAY];
#ifdef USE_SIMULATOR
      item->value = getGatewayIP();
#else
      item->value = const_cast<char*>(WiFi.gatewayIP().toString().c_str());
#endif
      lv_settings_add(item);

      item = &info_wifi_items[NETMASK];
#ifdef USE_SIMULATOR
      item->value = getSubnetMask();
#else
      item->value = const_cast<char*>(WiFi.subnetMask().toString().c_str());
#endif
      lv_settings_add(item);

      item = &info_wifi_items[DNS];
#ifdef USE_SIMULATOR
      item->value = getDNS();
#else
      item->value = const_cast<char*>(WiFi.dnsIP().toString().c_str());
#endif
      lv_settings_add(item);

      item = &info_wifi_items[CLIENT_NAME];
#ifdef USE_SIMULATOR
      item->value = getHostName();
#else
      item->value = const_cast<char*>(WiFi.getHostname());
#endif
      lv_settings_add(item);
    }
  }
}

static void wifi_event_cb(lv_obj_t * btn, lv_event_t e) {
  (void)btn;  //Unused

  //Get the caller item
  lv_settings_item_t * act_item = (lv_settings_item_t *)lv_event_get_data();

  if(e == LV_EVENT_REFRESH) {
    printf("wifi_event_cb: event = Refresh, id = %d, state = %d, value = %s, name = %s\n", act_item->id, act_item->state, act_item->value, act_item->name);

    if (act_item->id == _WIFI) {
      // Only show first element if wifiEnabled == false
      lv_settings_add(&wifi_items[WIFI_STATE]);
      uint32_t i;
      for(i = 1; wifi_items[i].type != LV_SETTINGS_TYPE_INV; i++) {
        lv_settings_add(&wifi_items[i]);
      }
    }
  } else if(e == LV_EVENT_VALUE_CHANGED) {
    printf("wifi_event_cb: event = Changed, id = %d, state = %d, value = %s, name = %s\n", act_item->id, act_item->state, act_item->value, act_item->name);
    if (act_item->id == _WIFI_AP) {
      useAPMode(act_item->state); /* 0 = No, 1 = Yes*/
    } else if (act_item->id == _WIFI_SSID) {
      int index = act_item->state;
      if (index == 1) { // 1 means "Search..."
        lv_show_preloader();
        LV_LOG_WARN("new scan started");
        // WiFi.scanNetworks called async
#ifdef USE_SIMULATOR
        scanNetworks();
#else
        WiFi.scanNetworks(true);
#endif
        ssidList = act_item;
        lv_task_create(wifiHelper, 1000, LV_TASK_PRIO_HIGH, NULL);
        // lv_settings_refr(act_item);
      } else {
        lv_obj_t * ddlist = lv_obj_get_child(act_item->cont, NULL);
        int index = lv_ddlist_get_selected(ddlist);
        printf("current index = %d\n", index);
        char buf[32];
        lv_ddlist_get_selected_str(ddlist, buf, sizeof(buf));
        printf("selected object = %s\n", buf);
        char * wifiName = subStr(buf, " (", 1);
        printf("Network name = %s\n", wifiName);
        _ssid = wifiName;

        bool wifiSecure = strstr(buf,")*");
        printf("Wifi is secured ? %s\n", wifiSecure ? "Yes" : "No");
        lv_settings_item_t * item;
        item = &wifi_items[WIFI_PASS];
        if (wifiSecure) {
          // Show Password input
          item->hidden = false;
          item->value = get_password_from_config(_ssid);
          printf("CURRENT PASSWORD = %s\n", item->value);
        } else {
          // Hide Password input
          item->hidden = true;
        }
        lv_settings_refr(item);

        /* Show Connect Button after change */
        item = &wifi_items[WIFI_CONNECT_BUTTON];
        item->hidden = false;
        item->name = "Disconnected";
#ifdef USE_SIMULATOR
        disconnectWiFi();
#else
        WiFi.disconnect();
#endif
        _wifiEnabled = false;
        lv_settings_refr(item);

        lv_hide_keyboard();
      }
    } else if (act_item->id == _WIFI && _wifiEnabled == false) {
      _wifiEnabled = true;

      lv_settings_item_t * item;
      for(int8_t y = 1; y<3; y++) {
      //  byte y = 2;
        item = &wifi_items[y];
        item->hidden = false;

        lv_settings_refr(item);
      }
    } else if (act_item->id == _WIFI && _wifiEnabled == true) {
      _wifiEnabled = false;

      lv_settings_item_t * item;
      for(int8_t y = 1; y<4; y++) {
      //  byte y = 2;
        item = &wifi_items[y];
        item->hidden = true;

        lv_settings_refr(item);
      }
    }
  } else if(e == LV_EVENT_CLICKED) {
    printf("wifi_event_cb: event = Clicked, id = %d, state = %d, value = %s, name = %s\n", act_item->id, act_item->state, act_item->value, act_item->name);
    if (act_item->id == _WIFI_BUTTON) {
      wifiConnectState = &wifi_items[WIFI_CONNECT_BUTTON];
      wifiConnectState->hidden = false;
      if (act_item->value != "Activate") {
        wifiConnectState->name = "Connecting...";
      } else {
        wifiConnectState->name = "Active";
      }
      lv_settings_refr(wifiConnectState);

      lv_show_preloader();

      // Get Password from keyboard
      char * pass = lv_get_text_from_keyboard();
      _ssid = "arcs";
      printf("---> Password from keyboard = '%s'\n", pass);
      printf("---> SSID = %s\n", _ssid);
      // If nothing is entered, get password from config
      if (strcmp(pass, "") == 0) {
        pass = get_password_from_config(_ssid);
      }

      if (strcmp(act_item->value, "Activate") == 0) {
#ifdef USE_SIMULATOR
    	connectSoftAP(_ssid, pass);
#else
        WiFi.softAP(_ssid, pass);
#endif

        wifiConnectState->value = "Deactivate";
        lv_settings_refr(wifiConnectState);

        lv_hide_preloader(100);
      } else if (strcmp(act_item->value, "Deactivate") == 0) {
#ifdef USE_SIMULATOR
    	disconnectSoftAP();
#else
    	WiFi.softAPdisconnect();
#endif

        wifiConnectState->name = "Deactivated";
        wifiConnectState->value = "Activate";
        lv_settings_refr(wifiConnectState);

        lv_hide_preloader(100);
      } else {
        // Set Mode explicit to station mode and disconnect actual connection
#ifdef USE_SIMULATOR
    	disconnectWiFi();
#else
        WiFi.mode(WIFI_STA);
        WiFi.disconnect();
        WiFi.begin(_ssid, pass);
#endif
        // lv_task_create(wifiConnect, 100, LV_TASK_PRIO_HIGH, NULL);
        lv_task_create(wifiConnect, 100, LV_TASK_PRIO_HIGH, _ssid);
      }
    }
  } else {
    printf("wifi_event_cb: event = Unknown, id = %d, state = %d, value = %s, name = %s\n", act_item->id, act_item->state, act_item->value, act_item->name);
  }
}

/*
// Settings menu
*/
#ifndef USE_SIMULATOR
TFT_eSPI tft = TFT_eSPI(); // Invoke custom library
static lv_disp_buf_t disp_buf;
static lv_color_t buf[LV_HOR_RES_MAX * 10];
#define LVGL_TICK_PERIOD 20
Ticker tick; /* timer for interrupt handler */
#endif

lv_obj_t * homeScreen;
lv_obj_t * headerScreen;

#define LV_COLOR_LIGHT_GREY LV_COLOR_MAKE(0xDB,0xDB,0xDB)

#ifdef ESP32
#include "FS.h"
#include "SPIFFS.h"
#include "ArduinoJson.h"

#define CONFIG_FILE "/config.json"

/* Only needed for debugging */
void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
  Serial.printf("Listing directory: %s\r\n", dirname);

  fs::File root = fs.open(dirname);
  if(!root){
    Serial.println("- failed to open directory");
    return;
  }
  if(!root.isDirectory()){
    Serial.println(" - not a directory");
    return;
  }

  fs::File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if(levels){
        listDir(fs, file.name(), levels -1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
  Serial.println("listDir done");
}

void deleteFile(fs::FS &fs, const char * path){
  Serial.printf("Deleting file: %s\r\n", path);
  if(fs.remove(path)){
      Serial.println("- file deleted");
  } else {
      Serial.println("- delete failed");
  }
}
/* Only needed for debugging */

void read_config() {
  fs::File file = SPIFFS.open(CONFIG_FILE);
  DynamicJsonDocument jsonBuffer(2048);
  deserializeJson(jsonBuffer, file);

  JsonArray ssids = jsonBuffer["ssid"];

  byte i = 0;
  Serial.println();
  for (JsonObject ssid : ssids) {
    lv_ssid_item_t * temp_item;

    if (ssid["mode"].as<int>() == 0) {
      // TODO: create the AP object
      // WIFI_AP_DEFAULT
      // WIFI_AP_PASS_DEFAULT
      strcpy(WIFI_AP_DEFAULT, ssid["ssid"].as<char*>());
      strcpy(WIFI_AP_PASS_DEFAULT, ssid["pass"].as<char*>());
    } else if (ssid["mode"].as<int>() == 1) {
      temp_item = &config_ssid_items[i];
      strcpy(temp_item->ssid, ssid["ssid"].as<char*>());
      strcpy(temp_item->pass, ssid["pass"].as<char*>());
      temp_item->secure = ssid["secure"].as<bool>();
      temp_item->valid = true;
      i++;
    }
  }
}

void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);

  fs::File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- frite failed");
  }
}

void write_config() {
  StaticJsonDocument<2048> doc;
  JsonObject root = doc.to<JsonObject>();
  JsonArray levels = root.createNestedArray("ssid");

  StaticJsonDocument<512> ap;
  JsonObject ssid_ap = ap.createNestedObject();

  // TODO: remove hardcoded ssid and pass for AP mode
  ssid_ap["mode"] = 0;
  ssid_ap["ssid"] = WIFI_AP_DEFAULT;
  ssid_ap["pass"] = WIFI_AP_PASS_DEFAULT;
  ssid_ap["secure"] = true;
  levels.add(ssid_ap);

  for (byte i = 0; i < SSID_LIST_SIZE; i++) {
    lv_ssid_item_t * ssid_item = &ssid_items[i];
    if (ssid_item->valid == true && (ssid_item->secure == true && strcmp(ssid_item->pass, "") != 0)) {
      printf("add to config: ssid = %s, pass = %s, secure = %s\n", ssid_item->ssid, ssid_item->pass, ssid_item->secure ? "Yes": "No");
      ssid_ap["mode"] = 1;
      ssid_ap["ssid"] = ssid_item->ssid;
      ssid_ap["pass"] = ssid_item->pass;
      ssid_ap["secure"] = ssid_item->secure;
      levels.add(ssid_ap);
    }
  }

  char output[2048];
  serializeJson(doc, output);
  Serial.println(output);

  writeFile(SPIFFS, CONFIG_FILE, output);

  read_config();

  lv_ssid_item_t * ssid_item;
  for (byte i = 0; i < SSID_LIST_SIZE; i++) {
    ssid_item = &config_ssid_items[i];

    if (ssid_item->valid == true) {
      printf("check config: ssid = %s, pass = %s, secure = %s\n", ssid_item->ssid, ssid_item->pass, ssid_item->secure ? "Yes": "No");
    }
  }

}
#else
void write_config() {
	LV_LOG_WARN("write_config called");
}
#endif

#ifndef USE_SIMULATOR
//------------------------------------------------------------------------------------------
void setup() {
  // Testing Filesystem -> only valid on a esp32
  // If we have an similiar board (e.g. arduino nano), we must save the ssid/pass to eeprom
#ifdef ESP32
#define FORMAT_SPIFFS_IF_FAILED true
  if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
    Serial.println("SPIFFS Mount Failed");
    return;
  }

/**
 * {
 *  "wifi_mode":"0", // 0 = AP, 1 = STA
 *  "ssid":"arcs",
 *  "pass":"tobedefined"
 *  "secure":"true"
 * }
 */
  // listDir(SPIFFS, "/", 0);

  // write_config();

  // deleteFile(SPIFFS, CONFIG_FILE);

  Serial.println();
  printf("WIFI_AP_DEFAULT = %s, WIFI_AP_PASS_DEFAULT = %s\n", WIFI_AP_DEFAULT, WIFI_AP_PASS_DEFAULT);
  read_config();
  printf("WIFI_AP_DEFAULT = %s, WIFI_AP_PASS_DEFAULT = %s\n", WIFI_AP_DEFAULT, WIFI_AP_PASS_DEFAULT);

  lv_ssid_item_t * ssid_item;
  for (byte i = 0; i < SSID_LIST_SIZE; i++) {
    // ssid_item = &ssid_items[i];
    ssid_item = &config_ssid_items[i];

    if (ssid_item->valid == true) {
      printf("add to config: ssid = %s, pass = %s, secure = %s\n", ssid_item->ssid, ssid_item->pass, ssid_item->secure ? "Yes": "No");
    }
  }
#endif
}
//------------------------------------------------------------------------------------------
#else
void lv_ex_settings_3(void)
{
	lv_theme_t *th = lv_theme_material_init(210, NULL);

    /*Try with different theme too*/
//    lv_theme_t *th = lv_theme_material_init(10, NULL);
//    lv_theme_t *th = lv_theme_night_init(40, NULL);

    lv_theme_set_current(th);


/*Add keyboard or mousewheel input devices if enabled*/
#if LV_SETTINGS_KEYBOARD
    keyboard_init();
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);          /*Basic initialization*/
    indev_drv.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv.read_cb = keyboard_read;         /*This function will be called periodically (by the library) to get the mouse position and state*/

#elif LV_SETTINGS_MOUSEWHEEL
    mousewheel_init();
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);          /*Basic initialization*/
    indev_drv.type = LV_INDEV_TYPE_ENCODER;
    indev_drv.read_cb = mousewheel_read;         /*This function will be called periodically (by the library) to get the mouse position and state*/
#endif

#if LV_SETTINGS_KEYBOARD || LV_SETTINGS_MOUSEWHEEL
    lv_indev_t * indev = lv_indev_drv_register(&indev_drv);

    lv_group_t * g = lv_group_create();
    lv_indev_set_group(indev, g);
    lv_settings_set_group(g);
#endif

    lv_settings_item_t * i;
    i = &wifi_items[WIFI_STATE];
    i->state = _wifiEnabled;
    if (_wifiEnabled) {
	  i->value = "Active";
    } else {
	  i->value = "Deactivated";
    }

    // Create the settings menu with a root item
    lv_settings_create(&root_item, root_event_cb);
}
#endif
