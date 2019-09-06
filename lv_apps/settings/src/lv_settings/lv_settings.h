/**
 * @file lv_settings.h
 *
 */

#ifndef LV_SETTINGS_H
#define LV_SETTINGS_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef enum {
    LV_SETTINGS_TYPE_LIST_BTN,
    LV_SETTINGS_TYPE_BTN,
    LV_SETTINGS_TYPE_SW,
    LV_SETTINGS_TYPE_DDLIST,
    LV_SETTINGS_TYPE_NUMSET,
    LV_SETTINGS_TYPE_SLIDER,

/* husky-koglhof */
    LV_SETTINGS_TYPE_ROOT,
    LV_SETTINGS_TYPE_TEXT,
    LV_SETTINGS_TYPE_PASS,
    LV_SETTINGS_TYPE_DESCR,
    LV_SETTINGS_TYPE_INV = 0xff,
}lv_settings_type_t;

typedef struct {
    lv_settings_type_t type;
    int16_t id;           /*actual id*/
    char * name;          /*Name or title of the item*/
    char * value;         /*The current value as string*/
    bool hidden;          /*Show or hide setting*/
    int32_t state;        /*The current state, e.g. slider's value, switch state as a number */
    lv_obj_t * cont;
    union {
        void * ptr;
        int32_t int32;
    }user_data;
}lv_settings_item_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
static lv_obj_t * kb;
char * lv_get_text_from_keyboard();

/**
 * Create a settings application
 * @param root_item descriptor of the settings button. For example:
 * `lv_settings_menu_item_t root_item = {.name = "Settings", .event_cb = root_event_cb};`
 * @return the created settings button
 */
lv_obj_t * lv_settings_create(lv_settings_item_t * root_item, lv_event_cb_t event_cb);

/**
 * Automatically add the item to a group to allow navigation with keypad or encoder.
 * Should be called before `lv_settings_create`
 * The group can be change at any time.
 * @param g the group to use. `NULL` to not use this feature.
 */
void lv_settings_set_group(lv_group_t * g);

void lv_hide_keyboard();

/**
 * Create a preloader object and show it
 */
void lv_show_preloader();

/**
 * Remove the preloader object
 */
void lv_hide_preloader(uint16_t delay);

/**
 * Create a new page ask `event_cb` to add the item with `LV_EVENT_REFRESH`
 * @param parent_item pointer to an item which open the the new page. Its `name` will be the title
 * @param event_cb event handler of the menu page
 */
void lv_settings_open_page(lv_settings_item_t * parent_item, lv_event_cb_t event_cb);

/**
 * Add a list element to the page. With `item->name` and `item->value` texts.
 * @param page pointer to a menu page created by `lv_settings_create_page`
 */
void lv_settings_add(lv_settings_item_t * item);

/**
 * Refresh an item's name value and state.
 * @param item pointer to a an `lv_settings_item _t` item.
 */
void lv_settings_refr(lv_settings_item_t * item);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_TEMPL_H*/
