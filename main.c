#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static Layer *s_canvas_layer;
static AppTimer *s_glow_timer = NULL;

static void glow_timer_callback(void *data) {
    text_layer_set_text_color(s_time_layer, GColorYellow);
    s_glow_timer = NULL;
}

static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
    (void)axis;
    (void)direction;

    text_layer_set_text_color(s_time_layer, GColorWhite);

    if (s_glow_timer) {
        app_timer_cancel(s_glow_timer);
    }
    
    s_glow_timer = app_timer_register(5000, glow_timer_callback, NULL);
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);
    GPoint center = grect_center_point(&bounds);
    
    int min_dimension = (bounds.size.w < bounds.size.h) ? bounds.size.w : bounds.size.h;
    int max_radius = min_dimension / 2;

    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);

    graphics_context_set_stroke_width(ctx, 2);
    graphics_context_set_stroke_color(ctx, GColorCeleste);
    graphics_draw_circle(ctx, center, max_radius * 55 / 100);

    graphics_context_set_stroke_width(ctx, 3);
    graphics_context_set_stroke_color(ctx, GColorPictonBlue);
    graphics_draw_circle(ctx, center, max_radius * 75 / 100);

    graphics_context_set_stroke_width(ctx, 2);
    graphics_context_set_stroke_color(ctx, GColorBlueMoon);
    graphics_draw_circle(ctx, center, max_radius * 95 / 100);

    // 3. Binäre Umrandung
    graphics_context_set_text_color(ctx, GColorLightGray);
    GFont binary_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);

    const char* binary_str_horizontal = "101011001010110100101011001010110100";

#if defined(PBL_ROUND)
    graphics_draw_text(ctx, binary_str_horizontal, binary_font, GRect(0, 15, bounds.size.w, 20), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    graphics_draw_text(ctx, binary_str_horizontal, binary_font, GRect(0, bounds.size.h - 35, bounds.size.w, 20), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
#else
    graphics_draw_text(ctx, binary_str_horizontal, binary_font, GRect(0, -2, bounds.size.w, 20), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    graphics_draw_text(ctx, binary_str_horizontal, binary_font, GRect(0, bounds.size.h - 16, bounds.size.w, 20), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);

    int char_h = 14;
    int num_chars = bounds.size.h / char_h;
    for (int i = 1; i < num_chars - 1; i++) {
        const char* c = (i % 2 == 0) ? "1" : "0";
        graphics_draw_text(ctx, c, binary_font, GRect(2, i * char_h, 10, 20), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
        graphics_draw_text(ctx, c, binary_font, GRect(bounds.size.w - 10, i * char_h, 10, 20), GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
    }
#endif
}

static void update_time() {
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    static char s_buffer[8];
    strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);

    text_layer_set_text(s_time_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    (void)tick_time;
    (void)units_changed;
    update_time();
}

static void main_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    s_canvas_layer = layer_create(bounds);
    layer_set_update_proc(s_canvas_layer, canvas_update_proc);
    layer_add_child(window_layer, s_canvas_layer);

    s_time_layer = text_layer_create(GRect(0, bounds.size.h / 2 - 28, bounds.size.w, 56));
    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_text_color(s_time_layer, GColorYellow); 
    text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
}

static void main_window_unload(Window *window) {
    text_layer_destroy(s_time_layer);
    layer_destroy(s_canvas_layer);
}

static void init() {
    s_main_window = window_create();
    window_set_background_color(s_main_window, GColorBlack);

    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload
    });

    window_stack_push(s_main_window, true);
    
    accel_tap_service_subscribe(accel_tap_handler);

    update_time();
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
    tick_timer_service_unsubscribe();
    accel_tap_service_unsubscribe();
    window_destroy(s_main_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
