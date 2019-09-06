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
    WIFI_AP_SSID,
    WIFI_PASS,
    WIFI_CONNECT_BUTTON,
}wifi_item_t;

static lv_settings_item_t wifi_items[] = {
  {.type = LV_SETTINGS_TYPE_SW, .id = _WIFI, .name = "WiFi State", .value = "Disabled", .hidden = false},
  {.type = LV_SETTINGS_TYPE_SW, .id = _WIFI_AP, .name = "Use AP", .value = "Disabled", .hidden = true},
  {.type = LV_SETTINGS_TYPE_DDLIST, .id = _WIFI_SSID, .name = "SSID", .value = WIFI_SSID_DEFAULT, .hidden = true},
  {.type = LV_SETTINGS_TYPE_TEXT, .id = _WIFI_AP_SSID, .name = "SSID", .value = WIFI_AP_DEFAULT, .hidden = true},
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

// Make this tasks global available, so we can cancel it
lv_task_t * t_networkScanner;
lv_task_t * t_cancelNetworkScan;
static void cancelNetworkScan(lv_task_t * task) {
	LV_LOG_WARN("cancelNetworkScan");
	if (t_networkScanner != NULL) {
		LV_LOG_WARN("networkScanner Task is active, delete it");
		lv_task_del(t_networkScanner);
	}
	if (t_cancelNetworkScan != NULL) {
		LV_LOG_WARN("cancelNetworkScan Task is active, delete it");
		lv_task_del(t_cancelNetworkScan);
	}
	lv_hide_preloader(500);
}

static void networkScanner(lv_task_t * task) {
#ifdef USE_SIMULATOR
	int16_t count = scanComplete();
#else
  // Get actual count
  int16_t count = WiFi.scanComplete();
#endif
  // if count == -1, scan is still running
  if (count >= 0) {
	LV_LOG_WARN("scan done");
    char wifiString[200];
    if (count == 0) {
    	LV_LOG_WARN("no networks found");
    } else {
      // Set default entries for ddlist
      sprintf(wifiString, "%s", WIFI_SSID_DEFAULT);
      /* Clean out all ssid_items from list */
      for (int32_t i = 0; i < SSID_LIST_SIZE; i++) {
        lv_ssid_item_t * ssid_item = &ssid_items[i];
        ssid_item->valid = false;
        strcpy(ssid_item->pass, "");
      }

      for (int32_t i = 0; i < count; ++i) {
        // Print SSID and RSSI for each network found
    	// e.g. One (-64)\nTwo (-32)*
    	// <SSID_NAME> (-<RSSI>)<SECURE_MODE>
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

      // Set actual ddlist entries
      // and select first entry "Search..."
      item->value = wifiString;
      item->state = 0;
      lv_settings_refr(ssidList);

      // Print out all entries
//      for (int32_t i = 0; i < SSID_LIST_SIZE; i++) {
//        lv_ssid_item_t * ssid_item = &ssid_items[i];
//        if (ssid_item->valid == true) {
//          printf("ssid = %s, pass = %s, secure = %s\n", ssid_item->ssid, ssid_item->pass, ssid_item->secure ? "Yes": "No");
//        }
//      }
    }

    // Delete this task as it is no longer needed
    LV_LOG_WARN("Delete Task t_networkScanner");
    lv_task_del(task);
    LV_LOG_WARN("Delete Task t_cancelNetworkScan");
  	lv_task_del(t_cancelNetworkScan);
    // Hide Preloader in 500ms cause we've found some entries
    lv_hide_preloader(500);
  }
}

static int32_t wifiCount = 0;
static char * wifiPass = "";

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

      lv_settings_item_t * item = &wifi_items[WIFI_CONNECT_BUTTON];
      item->value = "Disconnect";

      char * ssid = (char *)task->user_data;

      // Get Password from keyboard
      // char * pass = lv_get_text_from_keyboard();
      printf("---> Password from keyboard = '%s'\n", wifiPass);
      // If nothing is entered, get password from config
      if (strcmp(wifiPass, "") == 0) {
    	  wifiPass = get_password_from_config(ssid);
      }

      printf("searching for %s in ssid_items\n", ssid);
      for (int32_t i = 0; i < SSID_LIST_SIZE; i++) { // sizeof ssid_items
        lv_ssid_item_t * ssid_item = &ssid_items[i];
        printf("ssid = %s, pass = %s, secure = %s, valid = %s\n", ssid_item->ssid, ssid_item->pass, ssid_item->secure ? "Yes": "No", ssid_item->valid ? "Yes": "No");

        if (ssid_item->valid == true && strcmp(ssid, ssid_item->ssid) == 0) {
          // ssid_item->pass = pass;
          strcpy(ssid_item->pass, wifiPass);
          printf("Found valid wifi_item: ssid = %s, pass = %s\n", ssid, wifiPass);

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

static lv_obj_t * keyboard;

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
  printf("useAPMode called: %s\n", mode ? "True" : "False");
  /* If we are in AP Mode we must change some items */
  if (mode == true) {
    lv_settings_item_t * item;

    // Set AP label to enabled
    item = &wifi_items[WIFI_USE_AP];
    item->value = "Enabled";
    lv_settings_refr(item);

    // Hide ddlist in AP Mode
    item = &wifi_items[WIFI_SSID];
    item->hidden = true;
    lv_settings_refr(item);

    // show ap ssid text
    item = &wifi_items[WIFI_AP_SSID];
    _ssid = WIFI_AP_DEFAULT;
    item->value = WIFI_AP_DEFAULT;
    item->hidden = false;
    lv_settings_refr(item);

    // Hide Password in AP Mode
    item = &wifi_items[WIFI_PASS];
    item->hidden = true;
    lv_settings_refr(item);

    // Change password type to text
    item->type = LV_SETTINGS_TYPE_TEXT;
    item->value = WIFI_AP_PASS_DEFAULT;
    item->hidden = false;
    lv_settings_refr(item);

    // Deactivate Connect button
    // We do not register the overriden event handler, cause
    // it is allowed to create an AP without a password
    item = &wifi_items[WIFI_CONNECT_BUTTON];
    item->name = "Deactivated";
    item->value = "Activate";
    item->hidden = false;
    lv_settings_refr(item);

    lv_obj_t * btn = lv_obj_get_child(item->cont, NULL);
	lv_btn_set_state(btn, LV_BTN_STATE_REL);

    // for safety's sake, the keyboard is removed
    lv_hide_keyboard();
  } else {
	/* If we are in STA Mode we must change some items back from AP Mode */
	lv_settings_item_t * item;

    // Set AP label to disabled
	item = &wifi_items[WIFI_USE_AP];
    item->value = "Disabled";
    lv_settings_refr(item);

    // hide text in STA Mode
    item = &wifi_items[WIFI_AP_SSID];
    item->hidden = true;
    lv_settings_refr(item);

    // show ddlist ssid
    item = &wifi_items[WIFI_SSID];
    item->value = WIFI_SSID_DEFAULT;
    item->hidden = false;
    item->state = 0;
//    lv_settings_add(item);
    lv_settings_refr(item);

    // Show Password in Client Mode
    item = &wifi_items[WIFI_PASS];
    item->hidden = true;
    lv_settings_refr(item);

    // Change password type to password
    item->type = LV_SETTINGS_TYPE_PASS;
    item->value = WIFI_AP_PASS_DEFAULT;
    item->hidden = true;
//    lv_settings_add(item);
    lv_settings_refr(item);

    // Activate Connect button
    // Correct eventhander (the overriden one) is already registered
    item = &wifi_items[WIFI_CONNECT_BUTTON];
    item->name = "Deactivated";
    item->value = "Activate";
    item->hidden = true;
    lv_settings_refr(item);

    // for safety's sake, the keyboard is removed
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

static void showHideWifiSettings(bool visible) {
	lv_settings_item_t * act_item = (lv_settings_item_t *)lv_event_get_data();
	printf("!!!! showHideWifiSettings: visible = %s\n", visible ? "Yes": "No");

	if (visible) {
	  _wifiEnabled = true; // Used for ARCS

	  // Set Button Value to Activated
	  lv_settings_item_t * item;
	  act_item->value = "Activated";
	  lv_settings_refr(act_item);

	  // Show "Use AP" Switch
	  item = &wifi_items[WIFI_USE_AP];
	  item->hidden = false;
	  // Set AP Mode explicit to disabled
	  lv_obj_t * sw = lv_obj_get_child(item->cont, NULL);
	  lv_sw_off(sw, LV_ANIM_OFF);
	  // useAPMode(false);
	  // TODO: Not working
	  // item->state = false;
	  lv_settings_refr(item);

	  // Show "SSID" ddList
	  item = &wifi_items[WIFI_SSID];
	  item->hidden = false;
      // TODO: Remove hardcoded
      item->value = WIFI_SSID_DEFAULT;
      item->state = 0;

	  lv_settings_refr(item);

	  // Hide "Password" text
	  item = &wifi_items[WIFI_PASS];
	  item->hidden = true;
	  lv_settings_refr(item);

	  // Hide "Connect" Button
	  item = &wifi_items[WIFI_CONNECT_BUTTON];
	  item->hidden = true;
	  lv_settings_refr(item);
	} else {
	  _wifiEnabled = false; // Used for ARCS

	  lv_settings_item_t * item;
	  act_item->value = "Deactivated";
	  lv_settings_refr(act_item);

	  // Hide "Use AP" Switch
	  item = &wifi_items[WIFI_USE_AP];
	  item->hidden = true;
	  lv_settings_refr(item);

	  // Hide "SSID" ddList
	  item = &wifi_items[WIFI_SSID];
	  item->hidden = true;
	  lv_settings_refr(item);

	  // Hide "Password" text and clear
	  item = &wifi_items[WIFI_PASS];
	  item->hidden = true;
	  item->value = "";
	  lv_settings_refr(item);

	  // Hide "Connect" Button
	  item = &wifi_items[WIFI_CONNECT_BUTTON];
	  item->hidden = true;
	  lv_settings_refr(item);
	}
}

/* Override methods */
int8_t text_length = 0;
static char * passwd = "";

#if LV_USE_ANIMATION
static void kb_hide_anim_end(lv_anim_t * a)
{
    lv_obj_del(a->var);
    keyboard = NULL;
}
#endif

static void keyboard_event_cb(lv_obj_t * event_kb, lv_event_t event)
{
	LV_LOG_WARN("###################################### keyboard_event_cb called");
    /* Just call the regular event handler */
    lv_kb_def_event_cb(event_kb, event);

    // printf("EVENT: %d\n", event);
    // printf("passwd = %s\n", passwd);
    if (event == LV_EVENT_APPLY || event == LV_EVENT_DEFOCUSED) {
        // Store new password
        wifiPass = passwd;
    } else if (event == LV_EVENT_CANCEL) {
        // Clean out temp password
        passwd = "";
    }

    if(event == LV_EVENT_APPLY || event == LV_EVENT_CANCEL || event == LV_EVENT_DEFOCUSED) {
#if LV_USE_ANIMATION
        lv_anim_t a;
        a.var = keyboard;
        a.start = lv_obj_get_y(keyboard);
        a.end = LV_VER_RES;
        a.exec_cb = (lv_anim_exec_xcb_t)lv_obj_set_y;
        a.path_cb = lv_anim_path_linear;
        a.ready_cb = kb_hide_anim_end;
        a.act_time = 0;
        a.time = 300;
        a.playback = 0;
        a.playback_pause = 0;
        a.repeat = 0;
        a.repeat_pause = 0;
        lv_anim_create(&a);
#else
        lv_obj_del(keyboard);
        keyboard = NULL;
#endif
        // printf("Password: %s\n", wifiPass);
    }
}

static void pass_event_cb(lv_obj_t * ta, lv_event_t event) {
    if(event == LV_EVENT_CLICKED) {
        /* Focus on the clicked text area */
        if(keyboard != NULL) {
        	lv_kb_set_ta(keyboard, ta);
        } else {
            LV_LOG_ERROR("CREATE NEW KEYBOARD FROM pass_event_cb");
        	lv_obj_t * parent = lv_obj_get_parent(lv_obj_get_parent(ta));

        	keyboard = lv_kb_create(parent, NULL);
            lv_obj_set_pos(keyboard, 5, 90);
            lv_obj_set_event_cb(keyboard, keyboard_event_cb); /* Setting a custom event handler stops the keyboard from closing automatically */
            lv_obj_set_size(keyboard, LV_HOR_RES - 10, 140);
            lv_kb_set_ta(keyboard, ta); /* Focus it on one of the text areas to start */
            lv_kb_set_cursor_manage(keyboard, true); /* Automatically show/hide cursors on text areas */

 #if LV_USE_ANIMATION
            lv_anim_t a;
            a.var = keyboard;
            a.start = LV_VER_RES;
            a.end = lv_obj_get_y(keyboard);
            a.exec_cb = (lv_anim_exec_xcb_t)lv_obj_set_y;
            a.path_cb = lv_anim_path_linear;
            a.ready_cb = NULL;
            a.act_time = 0;
            a.time = 300;
            a.playback = 0;
            a.playback_pause = 0;
            a.repeat = 0;
            a.repeat_pause = 0;
            lv_anim_create(&a);
#endif
       }
    } else if(event == LV_EVENT_INSERT) {
        const char * str = lv_event_get_data();
        printf("1 str: '%s'\n", str);
        printf("2 val: %s\n", lv_ta_get_text(ta));
        printf("3 str: %d\n", str[0]);
        passwd = lv_ta_get_text(ta);
        if(str[0] == '\n') {
            lv_event_send(keyboard, LV_EVENT_APPLY, NULL);
        } else if (str[0] == 127) { // Backspace
        	text_length --;
        } else {
        	text_length ++;
        }
        /*
         * Change Connect Button
         * Active:   if more text_length > 0
         * Inactive: if text_length == 0
         *
         */
      	if (text_length > 0) {
      		lv_settings_item_t * item;
      		item = &wifi_items[WIFI_CONNECT_BUTTON];
            item->hidden = false;
            item->name = "Connect";

            lv_obj_t * btn = lv_obj_get_child(item->cont, NULL);
            lv_btn_set_state(btn, LV_BTN_STATE_REL);
      	} else {
      		lv_settings_item_t * item;
      		item = &wifi_items[WIFI_CONNECT_BUTTON];
            item->hidden = false;
            item->name = "Disconnected";

            lv_obj_t * btn = lv_obj_get_child(item->cont, NULL);
            lv_btn_set_state(btn, LV_BTN_STATE_INA);
      	}
    }
}
/* Override methods */

static void wifi_event_cb(lv_obj_t * btn, lv_event_t e) {
  (void)btn;  //Unused

  //Get the caller item
  lv_settings_item_t * act_item = (lv_settings_item_t *)lv_event_get_data();

  if(e == LV_EVENT_REFRESH) {
    printf("wifi_event_cb: event = Refresh, id = %d, state = %d, value = %s, name = %s\n", act_item->id, act_item->state, act_item->value, act_item->name);

    if (act_item->id == _WIFI) {
      // Only show switch button, is configured as "hidden = false" in wifi_items
      // for(i = 1; wifi_items[i].type != LV_SETTINGS_TYPE_INV; i++) {
      for(int8_t i = 0; wifi_items[i].type != LV_SETTINGS_TYPE_INV; i++) {
        lv_settings_add(&wifi_items[i]);
      }

      lv_settings_item_t * item = &wifi_items[WIFI_SSID];
      // TODO: Remove hardcoded
      item->value = WIFI_SSID_DEFAULT;
      item->state = 0;

      lv_settings_refr(item);

      // Override default event handler for password
      LV_LOG_WARN("---------> Override default event handler for password");
      // If no entry, Connect button is "disabled"
      // If entry, Connect button is "enabled" and event_handler is overriden
      item = &wifi_items[WIFI_PASS];
      // lv_obj_set_event_cb(lv_obj_get_child(item->cont, NULL), pass_event_cb);

      lv_obj_t * name = lv_obj_get_child_back(item->cont, NULL);
      lv_obj_t * value = lv_obj_get_child_back(item->cont, name);
      lv_obj_set_event_cb(value, pass_event_cb);

      lv_settings_refr(item);
    }
  } else if(e == LV_EVENT_VALUE_CHANGED) {
    printf("wifi_event_cb: event = Changed, id = %d, state = %d, value = %s, name = %s\n", act_item->id, act_item->state, act_item->value, act_item->name);

    // If AP Mode is changed
    if (act_item->id == _WIFI_AP) {
      useAPMode(act_item->state); /* 0 = No, 1 = Yes*/
    } else if (act_item->id == _WIFI_SSID) {
      int index = act_item->state;
      if (index == 0) { // 0 means "Select..."
    	  lv_settings_item_t * item;
    	  // Hide "Connect" Button
    	  item = &wifi_items[WIFI_CONNECT_BUTTON];
    	  item->hidden = true;
    	  lv_settings_refr(item);

      } else if (index == 1) { // 1 means "Search..."
        // Show Preloader while searching
    	lv_show_preloader();
        LV_LOG_WARN("new scan started");
#ifdef USE_SIMULATOR
        scanNetworks();
#else
        // WiFi.scanNetworks called async
        WiFi.scanNetworks(true);
#endif
        // Workaround: set ssidList hardcoded
        // so we can use it in networkScanner method
        ssidList = act_item;
        // call networkScanner (scan network)
        t_networkScanner = lv_task_create(networkScanner, 1000, LV_TASK_PRIO_HIGH, NULL);
        // Start a cancel NetworkScan Task after 5 secs
        // Will be canceled from networkScanner Task if we found some networks
        t_cancelNetworkScan = lv_task_create(cancelNetworkScan, 5000, LV_TASK_PRIO_HIGH, NULL);
        // Both tasks will be canceled in cancelNetworkScan if we didn't find networks within 5 secs
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

        printf("--------------------------> ...\n");
        /* Show Connect Button after change */
        item = &wifi_items[WIFI_CONNECT_BUTTON];
        item->hidden = false;
        item->name = "Disconnected";
        item->value = "Connect";

        if (wifiSecure) {
			lv_obj_t * btn = lv_obj_get_child(item->cont, NULL);
			lv_btn_set_state(btn, LV_BTN_STATE_INA);
        } else {
        	lv_obj_t * btn = lv_obj_get_child(item->cont, NULL);
        	lv_btn_set_state(btn, LV_BTN_STATE_REL);
        }
        lv_settings_refr(item);
#ifdef USE_SIMULATOR
        disconnectWiFi();
#else
        WiFi.disconnect();
#endif
        _wifiEnabled = false;
        lv_settings_refr(item);

        lv_hide_keyboard();
      }
    } else if (act_item->id == _WIFI && act_item->value == "Deactivated") {
      showHideWifiSettings(true);
    } else if (act_item->id == _WIFI && act_item->value == "Activated") {
      showHideWifiSettings(false);
    }
  } else if(e == LV_EVENT_CLICKED) {
    printf("wifi_event_cb: event = Clicked, id = %d, state = %d, value = %s, name = %s\n", act_item->id, act_item->state, act_item->value, act_item->name);
    if (act_item->id == _WIFI_BUTTON) {
      // TODO: Check if button is active or inactive
    	lv_settings_item_t * item;
	  item = &wifi_items[WIFI_CONNECT_BUTTON];
	  lv_obj_t * btn = lv_obj_get_child(item->cont, NULL);
	  lv_btn_state_t * btn_state = lv_btn_get_state(btn);

	  if (btn_state == LV_BTN_STATE_INA) {
      // if (act_item->value == "Connect" && act_item->name == "Disconnected") { // No password entered on a secure wifi, do nothing
    	  LV_LOG_WARN("Nothing todo, button is disabled");
    	  return;
      } else if (act_item->value == "Disconnect" && act_item->name == "Connected") { // STA mode connected, try to disconnect from these wifi
    	  LV_LOG_WARN("Disconnect from this wifi");
      }
      wifiConnectState = &wifi_items[WIFI_CONNECT_BUTTON];

      // Set correct text for button
      if (strcmp(act_item->value, "Activate") == 0) { // AP Mode
        wifiConnectState->name = "Active";
      } else if (act_item->value == "Disconnect" && act_item->name == "Connected") {
    	wifiConnectState->name = "Disconnect...";
      } else {
        wifiConnectState->name = "Connecting...";
      }
      lv_settings_refr(wifiConnectState);

      // Show preloader while connecting
      lv_show_preloader();

      printf("---> SSID = %s\n", _ssid);
      printf("---> Password from keyboard = '%s'\n", wifiPass);
      // If nothing is entered, get password from config
      if (strcmp(wifiPass, "") == 0) {
    	  wifiPass = get_password_from_config(_ssid);
        printf("---> No Password from keyboard, get Password from config = '%s'\n", wifiPass);
      }

      if (strcmp(act_item->value, "Activate") == 0) { // AP Mode
#ifdef USE_SIMULATOR
    	connectSoftAP(_ssid, wifiPass);
#else
        WiFi.softAP(_ssid, wifiPass);
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
    	  LV_LOG_WARN("---- STA MODE");
#ifdef USE_SIMULATOR
    	disconnectWiFi();
#else
        WiFi.mode(WIFI_STA);
        WiFi.disconnect();
        WiFi.begin(_ssid, pass);
#endif
        printf("########## value = %s, name = %s\n", act_item->value, act_item->name);
        if (act_item->value == "Disconnect" && act_item->name == "Disconnect...") {
        	LV_LOG_WARN("already disconnected, nothing todo");
        	lv_hide_preloader(500);

        	wifiConnectState->name = "Disconnected";
            wifiConnectState->value = "Connect";
            lv_settings_refr(wifiConnectState);
        } else {
        	LV_LOG_WARN("create task wifiConnect...");
        	lv_task_create(wifiConnect, 100, LV_TASK_PRIO_HIGH, _ssid);
        }
      }
      LV_LOG_WARN("HERE WE ARE");
      lv_hide_keyboard();

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
