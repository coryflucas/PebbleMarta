#include <pebble.h>

static Window *window;
static TextLayer *text_layer;

static const char *station_name;

enum {
  FETCH_TRAINS = 0x0,
  STATION_NAME = 0x1,
  TRAIN_INFO = 0x2,
};

static void fetch_trains(void) {
  Tuplet fetch_tupple = TupletInteger(FETCH_TRAINS, 1);
  Tuplet train_name_tupplet = TupletCString(STATION_NAME, station_name);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if(iter == NULL) {
    return;
  }

  dict_write_tuplet(iter, &fetch_tupple);
  dict_write_tuplet(iter, &train_name_tupplet);
  dict_write_end(iter);

  app_message_outbox_send();
}

static void window_load(Window *window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Station details load");
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  text_layer = text_layer_create( GRect(5, 40, bounds.size.w-10, 100));
  text_layer_set_text(text_layer, station_name);
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));

  fetch_trains();
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}

static void in_recieved_handler(DictionaryIterator *iter, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App message recieved");
  Tuple *train_tuple = dict_find(iter, TRAIN_INFO);

  if(train_tuple) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Train info: %s!", train_tuple->value->cstring);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "length: %d", strlen(train_tuple->value->cstring) );
  }
}

static void in_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App message dropped %d", reason);
}

static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App message failed to send %d", reason);
}

void station_details_init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  app_message_register_inbox_received(in_recieved_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_outbox_failed(out_failed_handler);
  app_message_open(256, 64);
}

void station_details_deinit(void) {
  window_destroy(window);
}

void station_details_show(const char *stationName) {
  station_name = stationName;
  window_stack_push(window, true);
}