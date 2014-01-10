#include <pebble.h>
#define TRAIN_INFO_LEN 256
#define MAX_TRAINS 16
#define TEXT_MSGS_LEN 32

typedef struct {
  char *name;
  char *direction;
  char description[14];
  int num_arrival_times;
  char *arrival_times[3];
} Line;

static Window *window;
static TextLayer *loading_layer;
static MenuLayer *menu_layer;

static const char *station_name;

static int num_lines = 0;
static Line lines[8];

static char train_info[TRAIN_INFO_LEN];

enum {
  FETCH_TRAINS = 0x0,
  STATION_NAME = 0x1,
  TRAIN_INFO = 0x2,
};

const char line_seperator = '|';
const char line_field_seperator = ';';
const char arrival_seperator = ',';

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

  num_lines = 0;
  while(pos) {
    lines[num_lines].name = parse_line_name(pos);
    strncpy(lines[num_lines].description, parse_line_name(pos), 5);
    strncat(lines[num_lines].description, " - ", 3);

    pos = move_to_next_token(pos, line_field_seperator);
    lines[num_lines].direction = parse_direction(pos);
    strncat(lines[num_lines].description, parse_direction(pos), 5);

    pos = move_to_next_token(pos, line_field_seperator);

    arrival_pos = pos;
    pos = move_to_next_token(pos, line_seperator);
    lines[num_lines].num_arrival_times = 0;
    do {
      lines[num_lines].arrival_times[lines[num_lines].num_arrival_times] = arrival_pos;
      arrival_pos = move_to_next_token(arrival_pos, arrival_seperator);

      lines[num_lines].num_arrival_times++;
    } while(arrival_pos);

    num_lines++;
  }

  menu_layer_reload_data(menu_layer);
  layer_set_hidden(text_layer_get_layer(loading_layer), true);
}

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return num_lines;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return num_lines > 0 ? lines[section_index].num_arrival_times : 0;
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static int16_t menu_get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  return 24;
}

static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  menu_cell_basic_header_draw(ctx,
                              cell_layer,
                              lines[section_index].description);
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  menu_cell_basic_draw(ctx,
                       cell_layer,
                       lines[cell_index->section].arrival_times[cell_index->row],
                       NULL,
                       NULL);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  loading_layer = text_layer_create(bounds);
  text_layer_set_text(loading_layer, "Loading...");
  text_layer_set_text_alignment(loading_layer, GTextAlignmentCenter);

  menu_layer = menu_layer_create(bounds);

  menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = menu_get_header_height_callback,
    .get_cell_height = menu_get_cell_height_callback,
    .draw_header = menu_draw_header_callback,
    .draw_row = menu_draw_row_callback,
  });

  menu_layer_set_click_config_onto_window(menu_layer, window);
  layer_add_child(window_layer, menu_layer_get_layer(menu_layer));
  layer_add_child(window_layer, text_layer_get_layer(loading_layer));

  fetch_trains();
}

static void window_unload(Window *window) {
  menu_layer_destroy(menu_layer);
  text_layer_destroy(loading_layer);
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