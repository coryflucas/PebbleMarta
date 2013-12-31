#include <pebble.h>
#include "StationList.h"

int main(void) {
  station_list_init();
  station_list_show();
  app_event_loop();
  station_list_deinit();
}
