#include "pebble.h"

#define REPEAT_INTERVAL_MS 50
  
#define PERIOD 1300
#define OUTOFBOUNDS 150

// This is a custom defined key for saving our count field
#define NUM_DRINKS_PKEY 1

// You can define defaults for values in persistent storage
#define NUM_DRINKS_DEFAULT 0

static Window *s_main_window;
Layer *window_layer;
static ActionBarLayer *s_action_bar;
static TextLayer *s_header_layer, *s_body_layer, *s_label_layer;
static TextLayer *s_text_layer;
static Layer *s_image_layer;
static GBitmap *s_icon_up, *s_icon_right, *s_icon_down, *s_icon_white;
static PropertyAnimation *s_prop_animation;
static int expectedGesture;
static int actualGesture = 0;
static AppTimer *s_progress_timer;
static int score = 0;

static int s_num_drinks = NUM_DRINKS_DEFAULT;

static void update_text() {
  static char s_body_text[18];
  snprintf(s_body_text, sizeof(s_body_text), "score: %u", score);
  text_layer_set_text(s_body_layer, s_body_text);
}

static void progress_timer_callback(void *data) {
  layer_remove_from_parent(text_layer_get_layer(s_text_layer));
  if (expectedGesture == actualGesture) {
    score++;
  }
  actualGesture = -1;
  expectedGesture = (rand() % 3);
  s_text_layer = text_layer_create(GRect(0, 20 + expectedGesture * 40, 16, 16));
  text_layer_set_text(s_text_layer, expectedGesture == 0 ? "U" : (expectedGesture == 1 ? "R" : "D" ));
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
  GRect to_rect = GRect(OUTOFBOUNDS, 20 + expectedGesture * 40, 16, 16);
  s_prop_animation = property_animation_create_layer_frame(text_layer_get_layer(s_text_layer), NULL, &to_rect);
  animation_set_duration((Animation*) s_prop_animation, PERIOD);
  animation_schedule((Animation*) s_prop_animation);
  s_progress_timer = app_timer_register(PERIOD, progress_timer_callback, NULL);
  update_text();
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  actualGesture = 0;
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  actualGesture = 1;
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  actualGesture = 2;
}

static void click_config_provider(void *context) {
  window_single_repeating_click_subscribe(BUTTON_ID_UP, REPEAT_INTERVAL_MS, up_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_SELECT, REPEAT_INTERVAL_MS, select_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, REPEAT_INTERVAL_MS, down_click_handler);
}

/*
static void layer_update_callback(Layer *layer, GContext* ctx) {
  // We make sure the dimensions of the GRect to draw into
  // are equal to the size of the bitmap--otherwise the image
  // will automatically tile. Which might be what *you* want.
  
  //graphics_draw_bitmap_in_rect(ctx, s_icon_up, GRect(s_icon_up_x, 20, 16, 16)); // 140 is probably max
}*/

static void main_window_load(Window *window) {
  window_layer = window_get_root_layer(window);

  s_action_bar = action_bar_layer_create();
  action_bar_layer_set_background_color(s_action_bar, GColorWhite);
  action_bar_layer_add_to_window(s_action_bar, window);
  action_bar_layer_set_click_config_provider(s_action_bar, click_config_provider);
  
  s_body_layer = text_layer_create(GRect(4, 120, 100, 16));
  layer_add_child(window_layer, text_layer_get_layer(s_body_layer));
  static char s_body_text[18];
  snprintf(s_body_text, sizeof(s_body_text), "score: %u", score);
  text_layer_set_text(s_body_layer, s_body_text);

  s_label_layer = text_layer_create(GRect(4, 120, 100, 16));
  text_layer_set_background_color(s_label_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(s_label_layer));
  
  expectedGesture = (rand() % 3);
  window_layer = window_get_root_layer(s_main_window);
  s_text_layer = text_layer_create(GRect(125, 20, 16, 16));
  text_layer_set_text(s_text_layer, "U");
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
  s_text_layer = text_layer_create(GRect(125, 60, 16, 16));
  text_layer_set_text(s_text_layer, "R");
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
  s_text_layer = text_layer_create(GRect(125, 100, 16, 16));
  text_layer_set_text(s_text_layer, "D");
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
  
  s_text_layer = text_layer_create(GRect(0, 20 + expectedGesture * 40, 16, 16));
  text_layer_set_text(s_text_layer, expectedGesture == 0 ? "U" : (expectedGesture == 1 ? "R" : "D" ));
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
  
  GRect to_rect = GRect(OUTOFBOUNDS, 20 + expectedGesture * 40, 16, 16);

  s_prop_animation = property_animation_create_layer_frame(text_layer_get_layer(s_text_layer), NULL, &to_rect);
  animation_set_duration((Animation*) s_prop_animation, PERIOD);
  animation_schedule((Animation*) s_prop_animation);
  s_progress_timer = app_timer_register(PERIOD, progress_timer_callback, NULL);
}

static void main_window_unload(Window *window) {
}

static void init() {
  s_icon_up = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_UP);
  s_icon_right = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_RIGHT);
  s_icon_down = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_DOWN);
  s_icon_white = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_WHITE);
  
  // Get the count from persistent storage for use if it exists, otherwise use the default
  s_num_drinks = persist_exists(NUM_DRINKS_PKEY) ? persist_read_int(NUM_DRINKS_PKEY) : NUM_DRINKS_DEFAULT;

  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
}

static void deinit() {
  // Save the count into persistent storage on app exit
  //persist_write_int(NUM_DRINKS_PKEY, s_num_drinks);
  text_layer_destroy(s_header_layer);
  text_layer_destroy(s_body_layer);
  text_layer_destroy(s_label_layer);
  gbitmap_destroy(s_icon_up);
  gbitmap_destroy(s_icon_right);
  gbitmap_destroy(s_icon_down);
  action_bar_layer_destroy(s_action_bar);
  layer_destroy(s_image_layer);
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}