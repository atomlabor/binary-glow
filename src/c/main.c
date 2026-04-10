#include <pebble.h>
#include <stdlib.h> 

static Window *s_main_window;
static TextLayer *s_time_layer, *s_time_shadow_layers[4];
static TextLayer *s_date_layer, *s_date_shadow_layers[4];
static GFont s_time_font;
static Layer *s_canvas_layer;

static AppTimer *s_glow_timer = NULL, *s_rapid_timer = NULL;
static bool s_is_glitching = false;
static int s_animation_tick = 0, s_color_scheme_index = 0, s_battery_level = 100;
static GColor s_ring_palettes[6][5], s_glitch_pal[6];

static GRect offset_rect(GRect rect, int dx, int dy) {
  return GRect(rect.origin.x + dx, rect.origin.y + dy, rect.size.w, rect.size.h);
}
static void battery_callback(BatteryChargeState state) {
  s_battery_level = state.charge_percent;
  if (s_canvas_layer) layer_mark_dirty(s_canvas_layer);
}
static void unobstructed_change_handler(AnimationProgress progress, void *context) {
  if (s_canvas_layer) layer_mark_dirty(s_canvas_layer);
  
  Layer *window_layer = window_get_root_layer(s_main_window);
  GRect full_bounds = layer_get_bounds(window_layer);
  GRect unobstructed_bounds = layer_get_unobstructed_bounds(window_layer);
  
  bool wide = (full_bounds.size.w > 160);
  int ty = unobstructed_bounds.size.h / 2 - (wide ? 34 : 22);
  
  GRect t_rect = GRect(0, ty, full_bounds.size.w, wide ? 70 : 44);
  GRect d_rect = GRect(0, ty - (wide ? 12 : 10), full_bounds.size.w, wide ? 24 : 18);
  
  layer_set_frame(text_layer_get_layer(s_time_layer), t_rect);
  layer_set_frame(text_layer_get_layer(s_date_layer), d_rect);
  
  int dx[4]={-2,2,-2,2}, dy[4]={-2,-2,2,2};
  for(int i=0; i<4; i++) {
    layer_set_frame(text_layer_get_layer(s_time_shadow_layers[i]), offset_rect(t_rect, dx[i], dy[i]));
    layer_set_frame(text_layer_get_layer(s_date_shadow_layers[i]), offset_rect(d_rect, dx[i], dy[i]));
  }
}
static void rapid_timer_callback(void *data) {
  if (s_is_glitching) {
    s_animation_tick++;
    text_layer_set_text_color(s_time_layer, s_glitch_pal[rand() % 6]);
    text_layer_set_text_color(s_date_layer, s_glitch_pal[rand() % 6]);
    layer_mark_dirty(s_canvas_layer);
    s_rapid_timer = app_timer_register(75, rapid_timer_callback, NULL);
  }
}
static void glow_timer_callback(void *data) {
  s_is_glitching = false;
  s_animation_tick = 0;
  text_layer_set_text_color(s_time_layer, GColorYellow);
  text_layer_set_text_color(s_date_layer, GColorYellow);
  layer_mark_dirty(s_canvas_layer);
  s_glow_timer = NULL;
}
static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
  s_is_glitching = true;
  s_animation_tick = 0;
  if (s_glow_timer) app_timer_cancel(s_glow_timer);
  if (s_rapid_timer) app_timer_cancel(s_rapid_timer);
  s_glow_timer = app_timer_register(5000, glow_timer_callback, NULL);
  rapid_timer_callback(NULL);
}
static void canvas_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GRect unobstructed_bounds = layer_get_unobstructed_bounds(layer);
  GPoint u_center = grect_center_point(&unobstructed_bounds);
  int max_radius = ((bounds.size.w < bounds.size.h) ? bounds.size.w : bounds.size.h) / 2;

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  int radii[10] = {45, 52, 62, 75, 90, 105, 120, 135, 150, 170};
  for (int i = 0; i < 10; i++) {
    int swing = s_is_glitching ? (sin_lookup((s_animation_tick * 8000 + i * 6000) % TRIG_MAX_ANGLE) * (4 + i * 2)) / TRIG_MAX_RATIO : 0;
    graphics_context_set_stroke_width(ctx, (i % 3 == 0) ? 2 : 1);
#if defined(PBL_BW)
    graphics_context_set_stroke_color(ctx, (i < 3) ? GColorWhite : (i < 7 ? GColorLightGray : GColorDarkGray));
#else
    graphics_context_set_stroke_color(ctx, s_ring_palettes[s_color_scheme_index][i % 5]);
#endif
    graphics_draw_circle(ctx, u_center, (max_radius * radii[i] / 100) + swing);
  }

  graphics_context_set_fill_color(ctx, GColorBlack);
#if defined(PBL_ROUND)
  graphics_draw_circle(ctx, u_center, max_radius - 12);
#else
  graphics_fill_rect(ctx, GRect(0, 0, bounds.size.w, 14), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(0, bounds.size.h - 14, bounds.size.w, 14), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(0, 0, 14, bounds.size.h), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(bounds.size.w - 14, 0, 14, bounds.size.h), 0, GCornerNone);
#endif

  graphics_context_set_text_color(ctx, PBL_IF_COLOR_ELSE(GColorYellow, GColorWhite));
  GFont b_font = fonts_get_system_font(FONT_KEY_GOTHIC_09);
  int minute_offset = localtime(&(time_t){time(NULL)})->tm_min;

#if defined(PBL_ROUND)
  for (int i = 0; i < 40; i++) {
    int32_t a = TRIG_MAX_ANGLE * i / 40;
    int x = u_center.x + (sin_lookup(a) * (max_radius - 10) / TRIG_MAX_RATIO);
    int y = u_center.y - (cos_lookup(a) * (max_radius - 10) / TRIG_MAX_RATIO);
    if (y < unobstructed_bounds.size.h) {
        graphics_draw_text(ctx, (i + minute_offset) % 2 == 0 ? "I" : "0", b_font, GRect(x-5, y-5, 10, 10), 0, GTextAlignmentCenter, NULL);
    }
  }
#else
  int char_w = 7; int char_h = 10;
  int num_x = bounds.size.w / char_w;
  float step_x = (float)(bounds.size.w - char_w) / (num_x - 1);
  for (int i = 0; i < num_x; i++) {
    const char* c = (i + minute_offset) % 2 == 0 ? "I" : "0";
    graphics_draw_text(ctx, c, b_font, GRect(i * step_x, -2, char_w, 14), 0, GTextAlignmentCenter, NULL);
    if (unobstructed_bounds.size.h == bounds.size.h) {
        graphics_draw_text(ctx, c, b_font, GRect(i * step_x, bounds.size.h-13, char_w, 14), 0, GTextAlignmentCenter, NULL);
    }
  }
  int num_y = unobstructed_bounds.size.h / char_h;
  float step_y = (float)(unobstructed_bounds.size.h - char_h) / (num_y - 1);
  for (int j = 1; j < num_y; j++) {
    const char* c = (j + minute_offset + 1) % 2 == 0 ? "I" : "0";
    graphics_draw_text(ctx, c, b_font, GRect(2, j * step_y - 1, 10, 14), 0, GTextAlignmentLeft, NULL);
    graphics_draw_text(ctx, c, b_font, GRect(bounds.size.w - 12, j * step_y - 1, 10, 14), 0, GTextAlignmentRight, NULL);
  }
#endif

  int bw = 30, bh = 3, bx = bounds.size.w / 2 - 15, by;
#if defined(PBL_ROUND)
  by = u_center.y + 28; 
#elif defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
  by = u_center.y + 33; 
#else
  by = u_center.y + (bounds.size.w > 160 ? 28 : 18); 
#endif
  graphics_context_set_stroke_color(ctx, GColorDarkGray);
  graphics_draw_rect(ctx, GRect(bx-1, by-1, bw+2, bh+2));
  graphics_context_set_fill_color(ctx, s_battery_level <= 20 ? GColorRed : PBL_IF_COLOR_ELSE(GColorYellow, GColorWhite));
  graphics_fill_rect(ctx, GRect(bx, by, (s_battery_level * bw) / 100, bh), 0, 0);
}

static void update_time() {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  static char t_buf[8], d_buf[16];
  strftime(t_buf, sizeof(t_buf), clock_is_24h_style() ? "%H:%M" : "%I:%M", t);
  strftime(d_buf, sizeof(d_buf), "%a %d.%m.", t);
  text_layer_set_text(s_time_layer, t_buf);
  text_layer_set_text(s_date_layer, d_buf);
  for(int i=0; i<4; i++) {
    text_layer_set_text(s_time_shadow_layers[i], t_buf);
    text_layer_set_text(s_date_shadow_layers[i], d_buf);
  }
  s_color_scheme_index = (t->tm_min / 5) % 6;
  if(s_canvas_layer) layer_mark_dirty(s_canvas_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) { update_time(); }

static void main_window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect b = layer_get_bounds(root);
  s_canvas_layer = layer_create(b);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(root, s_canvas_layer);

  bool wide = (b.size.w > 160);
  s_time_font = fonts_load_custom_font(resource_get_handle(wide ? RESOURCE_ID_FONT_SAIRA_58 : RESOURCE_ID_FONT_SAIRA_38));
  GFont d_font = fonts_get_system_font(wide ? FONT_KEY_GOTHIC_24_BOLD : FONT_KEY_GOTHIC_18_BOLD);

  int ty = b.size.h/2 - (wide ? 34 : 22);
  GRect t_rect = GRect(0, ty, b.size.w, wide ? 70 : 44);
  GRect d_rect = GRect(0, ty - (wide ? 12 : 10), b.size.w, wide ? 24 : 18);

  int dx[4]={-2,2,-2,2}, dy[4]={-2,-2,2,2};
  for(int i=0; i<4; i++) {
    s_time_shadow_layers[i] = text_layer_create(offset_rect(t_rect, dx[i], dy[i]));
    s_date_shadow_layers[i] = text_layer_create(offset_rect(d_rect, dx[i], dy[i]));
    text_layer_set_font(s_time_shadow_layers[i], s_time_font);
    text_layer_set_background_color(s_time_shadow_layers[i], GColorClear);
    text_layer_set_text_color(s_time_shadow_layers[i], GColorBlack);
    text_layer_set_text_alignment(s_time_shadow_layers[i], GTextAlignmentCenter);
    layer_add_child(root, text_layer_get_layer(s_time_shadow_layers[i]));
    
    text_layer_set_font(s_date_shadow_layers[i], d_font);
    text_layer_set_background_color(s_date_shadow_layers[i], GColorClear);
    text_layer_set_text_color(s_date_shadow_layers[i], GColorBlack);
    text_layer_set_text_alignment(s_date_shadow_layers[i], GTextAlignmentCenter);
    layer_add_child(root, text_layer_get_layer(s_date_shadow_layers[i]));
  }
  
  s_time_layer = text_layer_create(t_rect);
  s_date_layer = text_layer_create(d_rect);
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, PBL_IF_COLOR_ELSE(GColorYellow, GColorWhite));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(root, text_layer_get_layer(s_time_layer));

  text_layer_set_font(s_date_layer, d_font);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, PBL_IF_COLOR_ELSE(GColorYellow, GColorWhite));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  layer_add_child(root, text_layer_get_layer(s_date_layer));

  UnobstructedAreaHandlers handlers = { .change = unobstructed_change_handler };
  unobstructed_area_service_subscribe(handlers, NULL);
}

static void main_window_unload(Window *window) {
  unobstructed_area_service_unsubscribe();
  for(int i=0; i<4; i++) {
    text_layer_destroy(s_time_shadow_layers[i]);
    text_layer_destroy(s_date_shadow_layers[i]);
  }
  text_layer_destroy(s_time_layer); text_layer_destroy(s_date_layer);
  layer_destroy(s_canvas_layer); fonts_unload_custom_font(s_time_font);
}

static void init() {
  s_ring_palettes[0][0] = GColorCeleste; s_ring_palettes[0][1] = GColorPictonBlue; s_ring_palettes[0][2] = GColorBlueMoon; s_ring_palettes[0][3] = GColorDukeBlue; s_ring_palettes[0][4] = GColorOxfordBlue;
  s_ring_palettes[1][0] = GColorMelon; s_ring_palettes[1][1] = GColorSunsetOrange; s_ring_palettes[1][2] = GColorRed; s_ring_palettes[1][3] = GColorDarkCandyAppleRed; s_ring_palettes[1][4] = GColorBulgarianRose;
  s_ring_palettes[2][0] = GColorMintGreen; s_ring_palettes[2][1] = GColorScreaminGreen; s_ring_palettes[2][2] = GColorKellyGreen; s_ring_palettes[2][3] = GColorIslamicGreen; s_ring_palettes[2][4] = GColorDarkGreen;
  s_ring_palettes[3][0] = GColorRichBrilliantLavender; s_ring_palettes[3][1] = GColorVividViolet; s_ring_palettes[3][2] = GColorPurple; s_ring_palettes[3][3] = GColorIndigo; s_ring_palettes[3][4] = GColorImperialPurple;
  s_ring_palettes[4][0] = GColorPastelYellow; s_ring_palettes[4][1] = GColorYellow; s_ring_palettes[4][2] = GColorChromeYellow; s_ring_palettes[4][3] = GColorOrange; s_ring_palettes[4][4] = GColorWindsorTan;
  s_ring_palettes[5][0] = GColorElectricBlue; s_ring_palettes[5][1] = GColorCyan; s_ring_palettes[5][2] = GColorVividCerulean; s_ring_palettes[5][3] = GColorMidnightGreen; s_ring_palettes[5][4] = GColorDarkGreen;
  s_glitch_pal[0] = GColorWhite; s_glitch_pal[1] = GColorYellow; s_glitch_pal[2] = GColorChromeYellow; s_glitch_pal[3] = GColorRajah; s_glitch_pal[4] = GColorPastelYellow; s_glitch_pal[5] = GColorIcterine;

  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers){.load=main_window_load, .unload=main_window_unload});
  window_stack_push(s_main_window, true);
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  accel_tap_service_subscribe(accel_tap_handler);
  battery_state_service_subscribe(battery_callback);
  battery_callback(battery_state_service_peek());
  update_time();
}

static void deinit() { window_destroy(s_main_window); }
int main() { init(); app_event_loop(); deinit(); }