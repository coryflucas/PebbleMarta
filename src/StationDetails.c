#include <pebble.h>
#include "StringUtils.h"
#define TRAIN_INFO_LEN 256

static Window *window;
static TextLayer *text_layer;

static const char *station_name;
static char train_info[TRAIN_INFO_LEN];

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

const char line_seperator = '|';
const char line_field_seperator = ';';
const char arrival_seperator = ',';

static char *parse_line_name(char *line_name) {
  switch(*line_name) {
    case 'B':
      return "Blue";
    case 'G':
      return "Green";
    case 'R':
      return "Red";
    case 'Y':
      return "Gold";
  }
  return line_name;
}

static void handle_train_response(char *train_info) {
  char *line_info;
  char *next_line_info = train_info;
  char *line_name;
  char *direction;
  char *arrival_time;
  char *next_arrival_time;
  line_info = next_line_info;
  next_line_info = strtok2(line_info, line_seperator);
  while(*line_info != '\0') {
    line_name = line_info;
    direction = strtok2(line_name, line_field_seperator);

    next_arrival_time = strtok2(direction, line_field_seperator);
    do {
      arrival_time = next_arrival_time;
      next_arrival_time = strtok2(arrival_time, arrival_seperator);
      //TODO: put this in a string, set up text layer
      APP_LOG(APP_LOG_LEVEL_DEBUG, "%s line %s - %s min", parse_line_name(line_name), direction, arrival_time);
    } while(*next_arrival_time != '\0');

    line_info = next_line_info;
    next_line_info = strtok2(line_info, line_seperator);
  }
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
    strncpy(train_info, train_tuple->value->cstring, TRAIN_INFO_LEN);
    handle_train_response(train_info);
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
  app_message_open(TRAIN_INFO_LEN, 64);
}

void station_details_deinit(void) {
  window_destroy(window);
}

void station_details_show(const char *stationName) {
  station_name = stationName;
  window_stack_push(window, true);
}