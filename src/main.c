#include <pebble.h>
	
static Window *window;
static TextLayer *train_one_layer;
static TextLayer *train_two_layer;
static TextLayer *train_three_layer;
static TextLayer *train_four_layer;

static AppSync sync;
static uint8_t sync_buffer[64];

enum MessageKey {
	STATION_NAME_KEY = 0x1,
	TRAIN_ONE_KEY = 0x2,
	TRAIN_TWO_KEY = 0x3,
	TRAIN_THREE_KEY = 0x4,
	TRAIN_FOUR_KEY = 0x5,
};

static void get_trains_for_station(char *station_name) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Requesting info for train %s", station_name);
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	
	Tuplet value = TupletCString(STATION_NAME_KEY, station_name);
	dict_write_tuplet(iter, &value);
	
	app_message_outbox_send();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
	get_trains_for_station("Avondale");
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {

}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {

}

static void click_config_provider(void *context) {
	window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
	window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Tuple Changed: %s", new_tuple->value->cstring);
	switch (key) {
		case TRAIN_ONE_KEY:
			text_layer_set_text(train_one_layer, new_tuple->value->cstring);
			break;
		case TRAIN_TWO_KEY:
            text_layer_set_text(train_two_layer, new_tuple->value->cstring);
            break;
        case TRAIN_THREE_KEY:
            text_layer_set_text(train_three_layer, new_tuple->value->cstring);
            break;
        case TRAIN_FOUR_KEY:
            text_layer_set_text(train_four_layer, new_tuple->value->cstring);
            break;
	}
}

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	int height = 20;

	train_one_layer = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { bounds.size.w, height } });
	layer_add_child(window_layer, text_layer_get_layer(train_one_layer));

	train_two_layer = text_layer_create((GRect) { .origin = { 0, height }, .size = { bounds.size.w, height } });
	layer_add_child(window_layer, text_layer_get_layer(train_two_layer));

	train_three_layer = text_layer_create((GRect) { .origin = { 0, height * 2 }, .size = { bounds.size.w, height } });
	layer_add_child(window_layer, text_layer_get_layer(train_three_layer));

	train_four_layer = text_layer_create((GRect) { .origin = { 0, height * 3 }, .size = { bounds.size.w, height } });
	layer_add_child(window_layer, text_layer_get_layer(train_four_layer));

	
	Tuplet initial_values[] = {
		TupletCString(TRAIN_ONE_KEY, "loading..."),
	};
	
	app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
				  sync_tuple_changed_callback, sync_error_callback, NULL);
	get_trains_for_station("Avondale");
}

static void window_unload(Window *window) {
    text_layer_destroy(train_one_layer);
    text_layer_destroy(train_two_layer);
    text_layer_destroy(train_three_layer);
    text_layer_destroy(train_four_layer);
}

void init(void) {
	window = window_create();
	window_set_click_config_provider(window, click_config_provider);
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});

	const int inbound_size = 64;
	const int outbound_size = 64;
	app_message_open(inbound_size, outbound_size);

	const bool animated = true;
	window_stack_push(window, animated);
}

void deinit(void) {
	window_destroy(window);
}

int main(void) {
	init();
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);
	
	app_event_loop();
	deinit();
}
