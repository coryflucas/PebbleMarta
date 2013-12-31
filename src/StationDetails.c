#include <pebble.h>

static Window *window;
static TextLayer *text_layer;

static const char *station_name;

static void window_load(Window *window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Station details load");
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  text_layer = text_layer_create( GRect(5, 40, bounds.size.w-10, 100));
  text_layer_set_text(text_layer, station_name);
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void window_unload(Window *window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Station details unload");
  text_layer_destroy(text_layer);
}

void station_details_init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
}

void station_details_deinit(void) {
  window_destroy(window);
}

void station_details_show(const char *stationName) {
  station_name = stationName;
  window_stack_push(window, true);
}