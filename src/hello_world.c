#include "pebble.h"
  #include "phoneRouting.h"

  #define INVERT_COLORS

  #ifndef INVERT_COLORS
  #define COLOR_FOREGROUND GColorBlack
  #define COLOR_BACKGROUND GColorWhite
  #else
  #define COLOR_FOREGROUND GColorWhite
  #define COLOR_BACKGROUND GColorBlack
  #endif

static const int SEND_HELP_KEY = 0;
static const int SEND_HELP_VALUE = 1;
static const int SENT_TO_HELP = 1;
static const int FINISHED_SENDING = 2;

static Window *s_main_window; 
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static Layer *s_board_layer;
static Layer *window_layer;
static Layer* circle_layer1;
static Layer* circle_layer2;


static TextLayer *s_output_layer;

// ----- Message calls and receive from pebble to phone and vice versa
static void messageSend(int key, int value) {
  DictionaryIterator *iterator;
  app_message_outbox_begin(&iterator);
  dict_write_int(iterator, key, &value, sizeof(int), true);
  app_message_outbox_send();
}

void draw_circle1_update_proc(Layer *this_layer, GContext *ctx) {
  graphics_context_set_stroke_color(ctx, COLOR_FOREGROUND);
  graphics_draw_circle(ctx, GPoint(20,10), 3);
}

void draw_circle2_update_proc(Layer *this_layer, GContext *ctx) {
  graphics_context_set_stroke_color(ctx, COLOR_FOREGROUND);
  graphics_draw_circle(ctx, GPoint(30,10), 3);
}


static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Get the first pair
  Tuple *t = dict_read_first(iterator);

  // Process all pairs present
  while (t != NULL) {
    // Long lived buffer
    static char s_buffer[64];

    // Process this pair's key
    switch (t->key) {
      case 1:
        // Copy value and display8
        snprintf(s_buffer, sizeof(s_buffer), "Received '%s'", t->value->cstring);
      circle_layer1 = layer_create(GRect(0, 0, 144, 50));
        text_layer_set_text(s_output_layer, s_buffer);
          layer_set_update_proc(circle_layer1, draw_circle1_update_proc);
           layer_add_child(window_layer, circle_layer1);
        break;
      case 2:
      circle_layer2 = layer_create(GRect(0, 0, 144, 50));
          layer_set_update_proc(circle_layer2, draw_circle2_update_proc); 
          layer_add_child(window_layer, circle_layer2);
      break;
    }

    // Get next pair, if any
    t = dict_read_next(iterator);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

// ------


// ----- Click handlers for Pebble Watch
static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_output_layer, "Up pressed!");
  messageSend(SEND_HELP_KEY, SEND_HELP_VALUE);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_output_layer, "Select pressed!");
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_output_layer, "Down pressed!");
}

static void click_config_provider(void *context) {
  // Register the ClickHandlers
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}
//-----

//----- Drawing the a replica of the watch on home screen
void graphics_draw_line_wide(GContext *ctx, GPoint p0, GPoint p1) {
  for (int y_offset = 0; y_offset < 2; y_offset++) {
    graphics_draw_line(ctx, GPoint(p0.x, p0.y + y_offset), GPoint(p1.x, p1.y + y_offset));
  }
}

void draw_circle(GContext* ctx, GPoint center) {
  graphics_context_set_fill_color(ctx, COLOR_FOREGROUND);
  graphics_fill_circle(ctx, center, 3);
}



void board_layer_update_callback(Layer *layer, GContext* ctx) {
  graphics_context_set_stroke_color(ctx, COLOR_FOREGROUND);

  // Horizontal line
  graphics_draw_line_wide(ctx, GPoint(10, 89), GPoint(130, 89));
  draw_circle(ctx, GPoint(10, 10));
}

void update_date_text() {
  time_t now = time(NULL);
  const struct tm *current_time = localtime(&now);

  char *date_format;

  date_format = "%B %d";


  static char s_date_text[] = "November 00";
  strftime(s_date_text, sizeof(s_date_text), date_format, current_time);
  text_layer_set_text(s_date_layer, s_date_text);
}

void update_time_text() {
  time_t now = time(NULL);
  const struct tm *current_time = localtime(&now);

  char *time_format;
  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }

  static char s_time_text[] = "00:00";
  strftime(s_time_text, sizeof(s_time_text), time_format, current_time);

  // Handle lack of non-padded hour format string for twelve hour clock.
  if (!clock_is_24h_style() && (s_time_text[0] == '0')) {
    memmove(s_time_text, &s_time_text[1], sizeof(s_time_text) - 1);
  }
  text_layer_set_text(s_time_layer, s_time_text);

}

static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {
  update_time_text();
  update_date_text();
}

static void main_window_load(Window *window) {
  window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Init the layer that shows the board
  s_board_layer = layer_create(bounds); 
  layer_set_update_proc(s_board_layer, board_layer_update_callback); 
  layer_add_child(window_layer, s_board_layer);

  // Init the text layer used to show the time
  s_time_layer = text_layer_create(GRect(0, 85, 144, 50));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_text_color(s_time_layer, COLOR_FOREGROUND);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  //Init the date layer
  s_date_layer = text_layer_create(GRect(0, 60, 100, 80));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_text_color(s_date_layer, COLOR_FOREGROUND);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  // Create output TextLayer
  s_output_layer = text_layer_create(GRect(5, 0, 144, 50));
  text_layer_set_font(s_output_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_color(s_output_layer, COLOR_FOREGROUND);
  text_layer_set_text(s_output_layer, "No button pressed yet.");
  text_layer_set_background_color(s_output_layer, GColorClear);
  //text_layer_set_overflow_mode(s_output_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_layer, text_layer_get_layer(s_output_layer));

}
// -----

static void main_window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  layer_destroy(s_board_layer);
}

static void init() {
  s_main_window = window_create();
  window_set_background_color(s_main_window, COLOR_BACKGROUND);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });

  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Register app service
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  #ifdef PBL_SDK_2
    window_set_fullscreen(s_main_window, true);
  #endif

  window_set_click_config_provider(s_main_window, click_config_provider);

  window_stack_push(s_main_window, true);

  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);

  update_time_text();
}

static void deinit() {
  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
