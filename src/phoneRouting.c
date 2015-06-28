#include <pebble.h>
#include "phoneRouting.h"

static void send_int() {
  
  DictionaryIterator *iterator;
  app_message_outbox_begin(&iterator);

  int key = 0;
  int value = 1;
  dict_write_int(iterator, key, &value, sizeof(int), true);
  app_message_outbox_send();
  
  
}