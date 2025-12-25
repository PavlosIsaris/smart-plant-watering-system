/*Using LVGL with Arduino requires some extra steps:
 *Be sure to read the docs here: https://docs.lvgl.io/master/get-started/platforms/arduino.html  */

// Note: All required includes are now in the main .ino file
// This ensures proper include order when files are concatenated

/*To use the built-in examples and demos of LVGL uncomment the includes below respectively.
 *You also need to copy `lvgl/examples` to `lvgl/src/examples`. Similarly for the demos `lvgl/demos` to `lvgl/src/demos`.
 Note that the `lv_examples` library is for LVGL v7 and you shouldn't install it for this version (since LVGL v8)
 as the examples and demos are now part of the main LVGL library. */

// #include <examples/lv_examples.h>
// #include <demos/lv_demos.h>

// #define DIRECT_RENDER_MODE // Uncomment to enable full frame buffer

#define ROTATION 0
// #define ROTATION 1
// #define ROTATION 2
// #define ROTATION 3

#define GFX_BL 23

#define Touch_I2C_SDA 18
#define Touch_I2C_SCL 19
#define Touch_RST 20
#define Touch_INT 21

#define LEDC_FREQ 5000
#define LEDC_TIMER_10_BIT 10

void lvgl_plant_ui_init(lv_obj_t *parent);

Arduino_DataBus *bus = new Arduino_HWSPI(15 /* DC */, 14 /* CS */, 1 /* SCK */, 2 /* MOSI */);

Arduino_GFX *gfx = new Arduino_ST7789(
    bus, 22 /* RST */, 0 /* rotation */, false /* IPS */,
    172 /* width */, 320 /* height */,
    34 /*col_offset1*/, 0 /*uint8_t row_offset1*/,
    34 /*col_offset2*/, 0 /*row_offset2*/);

void lcd_reg_init(void)
{
  static const uint8_t init_operations[] = {
      BEGIN_WRITE,
      WRITE_COMMAND_8, 0x11, // 2: Out of sleep mode, no args, w/delay
      END_WRITE,
      DELAY, 120,

      BEGIN_WRITE,
      WRITE_C8_D16, 0xDF, 0x98, 0x53,
      WRITE_C8_D8, 0xB2, 0x23,

      WRITE_COMMAND_8, 0xB7,
      WRITE_BYTES, 4,
      0x00, 0x47, 0x00, 0x6F,

      WRITE_COMMAND_8, 0xBB,
      WRITE_BYTES, 6,
      0x1C, 0x1A, 0x55, 0x73, 0x63, 0xF0,

      WRITE_C8_D16, 0xC0, 0x44, 0xA4,
      WRITE_C8_D8, 0xC1, 0x16,

      WRITE_COMMAND_8, 0xC3,
      WRITE_BYTES, 8,
      0x7D, 0x07, 0x14, 0x06, 0xCF, 0x71, 0x72, 0x77,

      WRITE_COMMAND_8, 0xC4,
      WRITE_BYTES, 12,
      0x00, 0x00, 0xA0, 0x79, 0x0B, 0x0A, 0x16, 0x79, 0x0B, 0x0A, 0x16, 0x82,

      WRITE_COMMAND_8, 0xC8,
      WRITE_BYTES, 32,
      0x3F, 0x32, 0x29, 0x29, 0x27, 0x2B, 0x27, 0x28, 0x28, 0x26, 0x25, 0x17, 0x12, 0x0D, 0x04, 0x00, 0x3F, 0x32, 0x29, 0x29, 0x27, 0x2B, 0x27, 0x28, 0x28, 0x26, 0x25, 0x17, 0x12, 0x0D, 0x04, 0x00,

      WRITE_COMMAND_8, 0xD0,
      WRITE_BYTES, 5,
      0x04, 0x06, 0x6B, 0x0F, 0x00,

      WRITE_C8_D16, 0xD7, 0x00, 0x30,
      WRITE_C8_D8, 0xE6, 0x14,
      WRITE_C8_D8, 0xDE, 0x01,

      WRITE_COMMAND_8, 0xB7,
      WRITE_BYTES, 5,
      0x03, 0x13, 0xEF, 0x35, 0x35,

      WRITE_COMMAND_8, 0xC1,
      WRITE_BYTES, 3,
      0x14, 0x15, 0xC0,

      WRITE_C8_D16, 0xC2, 0x06, 0x3A,
      WRITE_C8_D16, 0xC4, 0x72, 0x12,
      WRITE_C8_D8, 0xBE, 0x00,
      WRITE_C8_D8, 0xDE, 0x02,

      WRITE_COMMAND_8, 0xE5,
      WRITE_BYTES, 3,
      0x00, 0x02, 0x00,

      WRITE_COMMAND_8, 0xE5,
      WRITE_BYTES, 3,
      0x01, 0x02, 0x00,

      WRITE_C8_D8, 0xDE, 0x00,
      WRITE_C8_D8, 0x35, 0x00,
      WRITE_C8_D8, 0x3A, 0x05,

      WRITE_COMMAND_8, 0x2A,
      WRITE_BYTES, 4,
      0x00, 0x22, 0x00, 0xCD,

      WRITE_COMMAND_8, 0x2B,
      WRITE_BYTES, 4,
      0x00, 0x00, 0x01, 0x3F,

      WRITE_C8_D8, 0xDE, 0x02,

      WRITE_COMMAND_8, 0xE5,
      WRITE_BYTES, 3,
      0x00, 0x02, 0x00,

      WRITE_C8_D8, 0xDE, 0x00,
      WRITE_C8_D8, 0x36, 0x00,
      WRITE_COMMAND_8, 0x21,
      END_WRITE,

      DELAY, 10,

      BEGIN_WRITE,
      WRITE_COMMAND_8, 0x29, // 5: Main screen turn on, no args, w/delay
      END_WRITE};
  bus->batchOperation(init_operations, sizeof(init_operations));
}

uint32_t screenWidth;
uint32_t screenHeight;
uint32_t bufSize;
lv_disp_draw_buf_t draw_buf;
lv_color_t *disp_draw_buf;
lv_disp_drv_t disp_drv;

#if LV_USE_LOG != 0
/* Serial debugging */
void my_print(const char *buf)
{
  Serial.printf(buf);
  Serial.flush();
}
#endif

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
#ifndef DIRECT_RENDER_MODE
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

#if (LV_COLOR_16_SWAP != 0)
  gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#else
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#endif
#endif // #ifndef DIRECT_RENDER_MODE

  lv_disp_flush_ready(disp_drv);
}

/*Read the touchpad*/
void touchpad_read_cb(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
  touch_data_t touch_data;
  // uint16_t touchpad_x[1] = { 0 };
  // uint16_t touchpad_y[1] = { 0 };
  uint8_t touchpad_cnt = 0;

  /* Read touch controller data */
  // esp_lcd_touch_read_data(touch_handle);
  bsp_touch_read();
  /* Get coordinates */
  // bool touchpad_pressed = esp_lcd_touch_get_coordinates(touch_handle, touchpad_x, touchpad_y, NULL, &touchpad_cnt, 1);
  bool touchpad_pressed = bsp_touch_get_coordinates(&touch_data);

  if (touchpad_pressed)
  {
    data->point.x = touch_data.coords[0].x;
    data->point.y = touch_data.coords[0].y;
    data->state = LV_INDEV_STATE_PRESSED;
    // printf("x:%03d, y:%03d\r\n", touchpad_x[0], touchpad_y[0]);
  }
  else
  {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

// Display and touch initialization
void displaySetup()
{
#ifdef DEV_DEVICE_INIT
  DEV_DEVICE_INIT();
#endif

  Serial.println("Initializing display and touch...");
  Serial.println("Arduino_GFX LVGL_Arduino_v8 example ");
  String LVGL_Arduino = String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.println(LVGL_Arduino);

  // Init Display
  if (!gfx->begin())
  {
    Serial.println("gfx->begin() failed!");
  }
  lcd_reg_init();
  gfx->setRotation(ROTATION);
  gfx->fillScreen(RGB565_BLACK);

#ifdef GFX_BL
  ledcAttach(GFX_BL, LEDC_FREQ, LEDC_TIMER_10_BIT);
  ledcWrite(GFX_BL, (1 << LEDC_TIMER_10_BIT) / 100 * 80);
#endif

  // Init touch device
  // touch_init(gfx->width(), gfx->height(), gfx->getRotation());
  Wire.begin(Touch_I2C_SDA, Touch_I2C_SCL);
  bsp_touch_init(&Wire, Touch_RST, Touch_INT, gfx->getRotation(), gfx->width(), gfx->height());
  lv_init();

#if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print); /* register print function for debugging */
#endif

  screenWidth = gfx->width();
  screenHeight = gfx->height();

#ifdef DIRECT_RENDER_MODE
  bufSize = screenWidth * screenHeight;
#else
  bufSize = screenWidth * 40;
#endif

#ifdef ESP32
#if defined(DIRECT_RENDER_MODE) && (defined(CANVAS) || defined(RGB_PANEL) || defined(DSI_PANEL))
  disp_draw_buf = (lv_color_t *)gfx->getFramebuffer();
#else  // !(defined(DIRECT_RENDER_MODE) && (defined(CANVAS) || defined(RGB_PANEL) || defined(DSI_PANEL)))
  disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize * 2, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  if (!disp_draw_buf)
  {
    // remove MALLOC_CAP_INTERNAL flag try again
    disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize * 2, MALLOC_CAP_8BIT);
  }
#endif // !(defined(DIRECT_RENDER_MODE) && (defined(CANVAS) || defined(RGB_PANEL) || defined(DSI_PANEL)))
#else  // !ESP32
  Serial.println("LVGL disp_draw_buf heap_caps_malloc failed! malloc again...");
  disp_draw_buf = (lv_color_t *)malloc(bufSize * 2);
#endif // !ESP32
  if (!disp_draw_buf)
  {
    Serial.println("LVGL disp_draw_buf allocate failed!");
  }
  else
  {
    lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, bufSize);

    /* Initialize the display */
    lv_disp_drv_init(&disp_drv);
    /* Change the following line to your display resolution */
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
#ifdef DIRECT_RENDER_MODE
    disp_drv.direct_mode = true;
#endif
    lv_disp_drv_register(&disp_drv);

    /* Initialize the (dummy) input device driver */
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touchpad_read_cb;
    lv_indev_drv_register(&indev_drv);

    /* Option 2: Try an example. See all the examples
     * online: https://docs.lvgl.io/master/examples.html
     * source codes: https://github.com/lvgl/lvgl/tree/e7f88efa5853128bf871dde335c0ca8da9eb7731/examples */
    // lv_example_btn_1();

    /* Option 3: Or try out a demo. Don't forget to enable the demos in lv_conf.h. E.g. LV_USE_DEMOS_WIDGETS*/
    // Initialize the plant watering UI instead of brightness demo
    // lvgl_brightness_ui_init(lv_scr_act());  // Old demo UI
    lvgl_plant_ui_init(lv_scr_act()); // New plant watering UI
    // lv_demo_widgets();
    // lv_demo_benchmark();
    // lv_demo_keypad_encoder();
    // lv_demo_music();
    // lv_demo_stress();
  }

  Serial.println("Display setup done");
}

// Display loop - handles LVGL timer and rendering
void displayLoop()
{
  lv_timer_handler(); /* let the GUI do its work */

#ifdef DIRECT_RENDER_MODE
#if defined(CANVAS) || defined(RGB_PANEL) || defined(DSI_PANEL)
  gfx->flush();
#else // !(defined(CANVAS) || defined(RGB_PANEL) || defined(DSI_PANEL))
#if (LV_COLOR_16_SWAP != 0)
  gfx->draw16bitBeRGBBitmap(0, 0, (uint16_t *)disp_draw_buf, screenWidth, screenHeight);
#else
  gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)disp_draw_buf, screenWidth, screenHeight);
#endif
#endif // !(defined(CANVAS) || defined(RGB_PANEL) || defined(DSI_PANEL))
#else  // !DIRECT_RENDER_MODE
#ifdef CANVAS
  gfx->flush();
#endif
#endif // !DIRECT_RENDER_MODE

  delay(5);
}

// UI elements
lv_obj_t *label_temperature;
lv_obj_t *label_humidity_air;
lv_obj_t *label_humidity_soil;
lv_obj_t *label_message;
lv_obj_t *btn_water;

// Water button event callback
void water_button_event_cb(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED)
  {
    Serial.println("Water button clicked");
    startWatering(); // Call function from main file
    lv_event_stop_bubbling(e);
  }
}

// Initialize the plant watering UI
void lvgl_plant_ui_init(lv_obj_t *parent)
{
  // Create main container
  lv_obj_t *container = lv_obj_create(parent);
  lv_obj_set_size(container, lv_pct(95), lv_pct(95));
  lv_obj_center(container);
  lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_top(container, 10, 0);
  lv_obj_set_style_pad_row(container, 8, 0);

  // Title
  lv_obj_t *label_title = lv_label_create(container);
  lv_label_set_text(label_title, "Plant Monitor");
  lv_obj_set_style_text_font(label_title, &lv_font_montserrat_16, 0);

  // Temperature label
  label_temperature = lv_label_create(container);
  lv_label_set_text(label_temperature, "Temp: --C");

  // Air humidity label
  label_humidity_air = lv_label_create(container);
  lv_label_set_text(label_humidity_air, "Air: --%");

  // Soil moisture label
  label_humidity_soil = lv_label_create(container);
  lv_label_set_text(label_humidity_soil, "Soil: --%");

  // Message/fact label
  label_message = lv_label_create(container);
  lv_label_set_text(label_message, "Ready");
  lv_label_set_long_mode(label_message, LV_LABEL_LONG_WRAP);
  lv_obj_set_width(label_message, lv_pct(90));
  lv_obj_set_style_text_align(label_message, LV_TEXT_ALIGN_CENTER, 0);

  // Water button
  btn_water = lv_btn_create(container);
  lv_obj_set_size(btn_water, lv_pct(80), 40);
  lv_obj_add_event_cb(btn_water, water_button_event_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *btn_label = lv_label_create(btn_water);
  lv_label_set_text(btn_label, "Water Plants");
  lv_obj_center(btn_label);
}

// Update home screen display with sensor data
void updateHomeScreenDisplay(int temp, int humidityAir, int humiditySoil)
{
  lv_label_set_text_fmt(label_temperature, "Temp: %dC", temp);
  lv_label_set_text_fmt(label_humidity_air, "Air: %d%%", humidityAir);
  lv_label_set_text_fmt(label_humidity_soil, "Soil: %d%%", humiditySoil);
}

// Update only soil moisture display
void updateSoilMoistureDisplay(int humiditySoil)
{
  lv_label_set_text_fmt(label_humidity_soil, "Soil: %d%%", humiditySoil);
}

// Display a message (like fun facts during watering)
void displayMessage(String message)
{
  lv_label_set_text(label_message, message.c_str());
}

// Update water button state (enable/disable and change text)
void updateWaterButtonState(bool isWatering)
{
  if (isWatering)
  {
    // Disable button and change text to "Watering..."
    lv_obj_add_state(btn_water, LV_STATE_DISABLED);
    lv_obj_t *btn_label = lv_obj_get_child(btn_water, 0);
    lv_label_set_text(btn_label, "Watering...");
  }
  else
  {
    // Enable button and restore text to "Water Plants"
    lv_obj_clear_state(btn_water, LV_STATE_DISABLED);
    lv_obj_t *btn_label = lv_obj_get_child(btn_water, 0);
    lv_label_set_text(btn_label, "Water Plants");
  }
}
