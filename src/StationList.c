#include <pebble.h>
#include "StationDetails.h"

static Window *window;
static MenuLayer *menu_layer;

const char *stations[] = {
  "Airport", "Arts Center", "Ashby", "Avondale", "Bankhead", "Brookhaven", "Buckhead", "Chamblee", "Civic Center", "College Park ", "Decatur", "Dome/GWCC/Philips/CNN", "Doraville", "Dunwoody", "East Lake", "East Point", "Edgewood/Candler Park", "Five Points", "Garnett", "Georgia State", "Hamilton", "Indian Creek", "Inman Park/Reynoldstown", "Kensington", "King Memorial", "Lakewood/Ft. McPherson", "Lenox ", "Lindbergh Center", "Medical Center", "Midtown", "North Avenue", "North Springs", "Oakland City", "Peachtree Center", "Sandy Springs", "Vine City", "West End", "West Lake"
};

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return 1;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
      return sizeof(stations)/sizeof(stations[0]);
    default:
      return 0;
  }
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
      menu_cell_basic_header_draw(ctx, cell_layer, "Stations");
      break;
  }
}

// This is the menu item draw callback where you specify what each item should look like
static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  // Determine which section we're going to draw in
  switch (cell_index->section) {
    case 0:
      //menu_cell_title_draw(ctx, cell_layer, stations[cell_index->row]);
      menu_cell_basic_draw(ctx, cell_layer, stations[cell_index->row], NULL, NULL);
      break;
  }
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  const char *selectedStation = stations[cell_index->row];
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Station selected: %s", selectedStation);
  station_details_show(selectedStation);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  menu_layer = menu_layer_create(bounds);

  menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = menu_get_header_height_callback,
    .draw_header = menu_draw_header_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback,
  });

  menu_layer_set_click_config_onto_window(menu_layer, window);
  layer_add_child(window_layer, menu_layer_get_layer(menu_layer));
}

static void window_unload(Window *window) {
  menu_layer_destroy(menu_layer);
}

void station_list_init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  station_details_init();
}

void station_list_deinit(void) {
  window_destroy(window);
  station_details_deinit();
}

void station_list_show(void) {
  window_stack_push(window, true);
}