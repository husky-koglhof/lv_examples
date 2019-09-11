/**
 * @file lv_ex_settings_4.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "../../src/lv_settings/lv_settings.h"
#include "lv_drivers/indev/keyboard.h"
#include "lv_drivers/indev/mousewheel.h"

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
static void info_menu_event_cb(lv_obj_t * btn, lv_event_t e);

/**********************
*  STATIC VARIABLES
**********************/
/*Declare items*/
static lv_settings_item_t root_item = {.name = "Settings", .value = ""};

static lv_settings_item_t main_menu_items[] = {
  {.type = LV_SETTINGS_TYPE_LIST_BTN, .name="Test", .value="Click me"},
  {.type = LV_SETTINGS_TYPE_INV},     //Mark the last item
};

static lv_settings_item_t sub_menu_items[] = {
  {.type = LV_SETTINGS_TYPE_SW, .name = "Switch", .value = "Disabled"},
  {.type = LV_SETTINGS_TYPE_LIST_BTN, .name="Not working", .value="Not working"},
  {.type = LV_SETTINGS_TYPE_BTN, .name="Test Button", .value="Test Button"},
  {.type = LV_SETTINGS_TYPE_INV},     //Mark the last item
};

/**********************
*      MACROS
**********************/

/**********************
*   GLOBAL FUNCTIONS
**********************/

/**********************
*   STATIC FUNCTIONS
**********************/
static void root_event_cb(lv_obj_t * btn, lv_event_t e) {
    (void)btn;  //Unused

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
  printf("main_menu_event_cb: event = %d, id = %d, state = %d, value = %s, name = %s\n", e, act_item->id, act_item->state, act_item->value, act_item->name);

  if(e == LV_EVENT_REFRESH) {
    uint32_t i;
    if (act_item->name == "Settings") {
      for(i = 0; main_menu_items[i].type != LV_SETTINGS_TYPE_INV; i++) {
        lv_settings_add(&main_menu_items[i]);
      }
	}
  } else if(e == LV_EVENT_CLICKED) {
    if (act_item->name == "Test") {
	  lv_settings_open_page(act_item, info_menu_event_cb);
    }
  }
}

static void info_menu_event_cb(lv_obj_t * btn, lv_event_t e) {
  (void)btn;  //Unused

  //Get the caller item/
  lv_settings_item_t * act_item = (lv_settings_item_t *)lv_event_get_data();
  printf("info_menu_event_cb: event = %d, id = %d, state = %d, value = %s, name = %s\n", e, act_item->id, act_item->state, act_item->value, act_item->name);

  if(e == LV_EVENT_REFRESH) {
    uint32_t i;
    if (act_item->name == "Test") {
      for(i = 0; sub_menu_items[i].type != LV_SETTINGS_TYPE_INV; i++) {
        lv_settings_add(&sub_menu_items[i]);
      }
    }
  } else if(e == LV_EVENT_VALUE_CHANGED) {
	  if (act_item->name == "Switch") {
		  lv_settings_item_t * working_item = &sub_menu_items[1]; // _WORKING
		  lv_settings_item_t * not_working_item = &sub_menu_items[2]; // _NOT_WORKING
		  if (act_item->state == 0) {
			  working_item->name = "On";
			  working_item->value = "On";

			  not_working_item->name = "On";
			  not_working_item->value = "Test3";
		  } else {
			  working_item->name = "Off";
			  working_item->value = "Test2";

			  not_working_item->name = "Off";
			  not_working_item->value = "Test4";
		  }
		  lv_settings_refr(working_item);
		  lv_settings_refr(not_working_item);
	  }
  }
}

void lv_ex_settings_4(void)
{
	lv_theme_t *th = lv_theme_material_init(210, NULL);
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

    // Create the settings menu with a root item
    lv_settings_create(&root_item, root_event_cb);
}
