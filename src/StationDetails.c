#include <pebble.h>
#define TRAIN_INFO_LEN 256
#define MAX_TRAINS 16
#define TEXT_MSGS_LEN 32

typedef struct {
  char *name;
  char *direction;
  char *arrival_times[3];
} Line;

static Window *window;
static ScrollLayer *scroll_layer;
static TextLayer *text_layers[MAX_TRAINS];

static const char *station_name;
static Line lines[8];

static char train_info[TRAIN_INFO_LEN];
static char text_messages[MAX_TRAINS][TEXT_MSGS_LEN];

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

static char *parse_direction(char *direction) {
  switch(*direction) {
    case 'E':
      return "East";
    case 'W':
      return "West";
    case 'N':
      return "North";
    case 'S':
      return "South";
  }
  return direction;
}

static char *move_to_next_token(char *source, char seperator) {
  char *pos = strchr(source, seperator);
  if(pos) {
    *pos = '\0';
    pos++;
  }
  return pos;
}

static void handle_train_response(char *train_info) {
  char *pos = train_info;
  char *arrival_pos;

  int i = 0;
  int line_idx = 0;
  int arrival_idx = 0;
  while(pos) {
    lines[line_idx].name = pos;
    pos = move_to_next_token(pos, line_field_seperator);
    lines[line_idx].direction = pos;
    pos = move_to_next_token(pos, line_field_seperator);

    arrival_pos = pos;
    pos = move_to_next_token(pos, line_seperator);
    arrival_idx = 0;
    do {
      lines[line_idx].arrival_times[arrival_idx] = arrival_pos;
      arrival_pos = move_to_next_token(arrival_pos, arrival_seperator);

      snprintf(text_messages[i], TEXT_MSGS_LEN, "%s line %s: %s min",
        parse_line_name(lines[line_idx].name),
        parse_direction(lines[line_idx].direction),
        lines[line_idx].arrival_times[arrival_idx]);
      layer_mark_dirty(text_layer_get_layer(text_layers[i]));
      i++;
      arrival_idx++;
    } while(arrival_pos);

    line_idx++;
  }

  GRect bounds = layer_get_bounds(window_get_root_layer(window));
  scroll_layer_set_content_size(scroll_layer, GSize(bounds.size.w, i * 16));

  for(; i < MAX_TRAINS; i++) {
    text_messages[i][0] = '\0';
  }
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  scroll_layer = scroll_layer_create(bounds);
  scroll_layer_set_click_config_onto_window(scroll_layer, window);
  layer_add_child(window_layer, scroll_layer_get_layer(scroll_layer));

  for(int i = 0; i < MAX_TRAINS; i++) {
    text_messages[i][0] = '\0';
    if(i == 0 ) {
      strncpy(text_messages[0], "Loading...", TEXT_MSGS_LEN);
    }
    text_layers[i] = text_layer_create( GRect(1, i * 16, bounds.size.w - 1, 16));
    text_layer_set_text(text_layers[i], text_messages[i]);
    scroll_layer_add_child(scroll_layer, text_layer_get_layer(text_layers[i]));
  }

  fetch_trains();
}

static void window_unload(Window *window) {
  scroll_layer_destroy(scroll_layer);
  for(int i = 0; i < MAX_TRAINS; i++) {
    text_layer_destroy(text_layers[i]);
  }
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