#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <Arduino_GFX.h>
#include <../lvgl/lvgl.h>
#include <TAMC_GT911.h>

/*===================TouchConfig====================*/
#define TOUCH_SDA 19
#define TOUCH_SCL 20
#define TOUCH_INT 18
#define TOUCH_RST 38

#define TOUCH_MAP_X1 800
#define TOUCH_MAP_X2 0
#define TOUCH_MAP_Y1 480
#define TOUCH_MAP_Y2 0

int start_x = 30;
int start_y = 10;

char *btn_name[] = {"ZERO - DONT USE", "Button 11", "Button 12", "Button 13",
                     "Button 14", "Button 15", "Button 16"};


int touch_last_x = 0;  // *1.65 for 800
int touch_last_y = 0;  // *1.75 for 480
int touch_last_RightX = 0;
int touch_last_RightY = 0;

void Calibration() {
    float multiplicadorX = 1.65;
    float multiplicadorY = 1.75;
    float multiplicadorNovoX;
    float multiplicadorNovoY;

    multiplicadorNovoX = (800 * multiplicadorX) / TOUCH_MAP_X1;
    multiplicadorNovoY = (480 * multiplicadorY) / TOUCH_MAP_Y1;

    touch_last_RightX = touch_last_x * multiplicadorNovoX;
    touch_last_RightY  = touch_last_y * multiplicadorNovoY;
}

TAMC_GT911 ts = TAMC_GT911(TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST, 
max(TOUCH_MAP_X1, TOUCH_MAP_X2), max(TOUCH_MAP_Y1, TOUCH_MAP_Y2));
/*===================TouchConfig====================*/

static lv_disp_draw_buf_t draw_buf;
static lv_disp_drv_t disp_drv;
static lv_color_t *disp_draw_buf;
static uint32_t screenWidth;
static uint32_t screenHeight;
static unsigned long last_ms;


#define GFX_BL DF_GFX_BL // default backlight pin, you may replace DF_GFX_BL to actual backlight pin
#define TFT_BL 2 // LED K

Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
    40 /* DE */, 41 /* VSYNC */, 39 /* HSYNC */, 42 /* PCLK */,
    45 /* R0 */, 48 /* R1 */, 47 /* R2 */, 21 /* R3 */, 14 /* R4 */,
    5 /* G0 */, 6 /* G1 */, 7 /* G2 */, 15 /* G3 */, 16 /* G4 */, 4 /* G5 */,
    8 /* B0 */, 3 /* B1 */, 46 /* B2 */, 9 /* B3 */, 1 /* B4 */,
    0 /* hsync_polarity */, 8 /* hsync_front_porch */, 4 /* hsync_pulse_width */, 8 /* hsync_back_porch */,
    0 /* vsync_polarity */, 8 /* vsync_front_porch */, 4 /* vsync_pulse_width */, 8 /* vsync_back_porch */,
    0 /* pclk_active_neg */, 16000000 /* prefer_speed */);
Arduino_RGB_Display *gfx = new Arduino_RGB_Display(
    800 /* width */, 480 /* height */, rgbpanel);

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);

  lv_disp_flush_ready(disp);
}

void touch_init()
{
    Wire.begin(TOUCH_SDA, TOUCH_SCL);
    ts.begin();
    ts.setRotation(ROTATION_NORMAL);
}

bool touch_touched()
{
    ts.read();
    if (ts.isTouched)
    {

        touch_last_x = map(ts.points[0].x, TOUCH_MAP_X1, TOUCH_MAP_X2, 0, 800 - 1);
        touch_last_y = map(ts.points[0].y, TOUCH_MAP_Y1, TOUCH_MAP_Y2, 0, 480 - 1);
        Calibration();

        return true;
    }
    else
    {
        return false;
    }
}


static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");
    }
    else if(code == LV_EVENT_VALUE_CHANGED) {
        LV_LOG_USER("Toggled");
    }
}


static void my_event_cb(lv_event_t * event)
{
    static uint32_t cnt = 1;
    lv_obj_t * btn = lv_event_get_target(event);
    lv_obj_t * label = lv_obj_get_child(btn, 0);
    //lv_label_set_text_fmt(label,"%" LV_PRIu32, cnt);
    lv_label_set_text_fmt(label,"%" LV_PRIu32, label);
    cnt++;
}

void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    if (touch_touched())
    {
      data->state = LV_INDEV_STATE_PR;

      /*Set the coordinates*/
      data->point.x = touch_last_RightX;
      data->point.y = touch_last_RightY;
    }
    else
    {
      data->state = LV_INDEV_STATE_REL;
    }
}

LV_IMG_DECLARE(img_cogwheel_argb);
LV_IMG_DECLARE(cogwheel);
LV_IMG_DECLARE(cogwheel2);

void lv_ex_img_1(void)
{
    start_x = 70;  // Across
    start_y = 40;  // Down
    lv_obj_t * img1 = lv_img_create(lv_scr_act());
    lv_img_set_src(img1, &img_cogwheel_argb);
    //lv_obj_align(img1, LV_ALIGN_CENTER, 400, 40);
    lv_obj_set_x(img1, start_x);
    lv_obj_set_y(img1, start_y);

    // lv_obj_t * img2 = lv_img_create(lv_scr_act());
    // lv_img_set_src(img2, LV_SYMBOL_OK "Accept");
    // lv_obj_set_x(img2, 20);
    // lv_obj_set_y(img2, 200);

    // Little CogWheel
    lv_obj_t * img3 = lv_img_create(lv_scr_act());
    lv_img_set_src(img3, &cogwheel);
    lv_obj_set_x(img3, 20);
    lv_obj_set_y(img3, 400);

    // start_x = 270;  // Across
    // start_y = 40;  // Down
    // lv_obj_t * img4 = lv_img_create(lv_scr_act());
    // lv_img_set_src(img4, &cogwheel2);
    // lv_obj_set_x(img4, start_x);
    // lv_obj_set_y(img4, start_y);
}

void lv_example_imgbtn_1(void)
{
    LV_IMG_DECLARE(imgbtn_left);
    LV_IMG_DECLARE(imgbtn_right);
    LV_IMG_DECLARE(imgbtn_mid);

    /*Create a transition animation on width transformation and recolor.*/
    static lv_style_prop_t tr_prop[] = {LV_STYLE_TRANSFORM_WIDTH, LV_STYLE_IMG_RECOLOR_OPA};
    static lv_style_transition_dsc_t tr;
    lv_style_transition_dsc_init(&tr, tr_prop, lv_anim_path_linear, 200, 0, NULL);

    static lv_style_t style_def;
    lv_style_init(&style_def);
    lv_style_set_text_color(&style_def, lv_color_white());
    lv_style_set_transition(&style_def, &tr);

    /*Darken the button when pressed and make it wider*/
    static lv_style_t style_pr;
    lv_style_init(&style_pr);
    lv_style_set_img_recolor_opa(&style_pr, LV_OPA_30);
    lv_style_set_img_recolor(&style_pr, lv_color_black());
    lv_style_set_transform_width(&style_pr, 20);

    /*Create an image button*/
    lv_obj_t * imgbtn1 = lv_imgbtn_create(lv_scr_act());
    lv_imgbtn_set_src(imgbtn1, LV_IMGBTN_STATE_RELEASED, &imgbtn_left, &imgbtn_mid, &imgbtn_right);
    lv_obj_add_style(imgbtn1, &style_def, 0);
    lv_obj_add_style(imgbtn1, &style_pr, LV_STATE_PRESSED);

    lv_obj_align(imgbtn1, LV_ALIGN_CENTER, 0, 0);

    /*Create a label on the image button*/
    lv_obj_t * label = lv_label_create(imgbtn1);
    lv_label_set_text(label, "Button");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, -4);
}


void setup()
{
  Serial.begin(115200);
  Serial.println("TAMC_GT911 Example: Ready");
  // Init Display
  gfx->begin();
  gfx->fillScreen(BLACK);

  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);

  lv_init();
  touch_init();

  screenWidth = gfx->width();
  screenHeight = gfx->height(); 

  disp_draw_buf = (lv_color_t *)heap_caps_malloc(sizeof(lv_color_t) * screenWidth *32, MALLOC_CAP_32BIT);

  if (!disp_draw_buf)

  {
    Serial.println("LVGL disp_draw_buf allocate failed!");
  }
  else
  {
    lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, 800 * 32);

    /* Initialize the display */
      lv_disp_drv_init(&disp_drv);
      /* Change the following line to your display resolution */
      disp_drv.hor_res = screenWidth;
      disp_drv.ver_res = screenHeight;
      disp_drv.flush_cb = my_disp_flush;
      disp_drv.draw_buf = &draw_buf;
      lv_disp_drv_register(&disp_drv);

      /* Initialize the (dummy) input device driver */
      static lv_indev_drv_t indev_drv;
      lv_indev_drv_init(&indev_drv);
      indev_drv.type = LV_INDEV_TYPE_POINTER;
      lv_indev_drv_register(&indev_drv);
      indev_drv.read_cb = my_touchpad_read;

    /* Create lvgl objs */

    lv_obj_t * label;

    // ##################### BUTTONS #######################//
    // Button 1,1
    start_x = 30;  // Across
    start_y = 10;  // Down
    lv_obj_t * btn11 = lv_btn_create(lv_scr_act());
    //lv_obj_add_event_cb(btn11, event_handler, LV_EVENT_ALL, NULL);
    //lv_obj_align(btn11, LV_ALIGN_CENTER, 0, -40);
    lv_obj_set_x(btn11, start_x);
    lv_obj_set_y(btn11, start_y);
    lv_obj_set_size(btn11, 180, 150);
    lv_obj_add_flag(btn11, LV_OBJ_FLAG_CHECKABLE);
    label = lv_label_create(btn11);
    lv_label_set_text(label, btn_name[1]);
    lv_obj_add_event_cb(btn11, my_event_cb, LV_EVENT_CLICKED, &start_x);

    // Button 1,2
    start_x = 30 + 190;
    lv_obj_t * btn12 = lv_btn_create(lv_scr_act());
    lv_obj_set_x(btn12, start_x);
    lv_obj_set_y(btn12, start_y);
    lv_obj_set_size(btn12, 180, 150);
    lv_obj_add_flag(btn12, LV_OBJ_FLAG_CHECKABLE);
    label = lv_label_create(btn12);
    lv_label_set_text(label, btn_name[2]);
    lv_obj_add_event_cb(btn12, my_event_cb, LV_EVENT_CLICKED, &start_x);

    // Button 1,3
    start_x = 30 + 380;
    lv_obj_t * btn13 = lv_btn_create(lv_scr_act());
    lv_obj_set_x(btn13, start_x);
    lv_obj_set_y(btn13, start_y);
    lv_obj_set_size(btn13, 180, 150);
    lv_obj_add_flag(btn13, LV_OBJ_FLAG_CHECKABLE);
    label = lv_label_create(btn13);
    lv_label_set_text(label, btn_name[3]);
    lv_obj_add_event_cb(btn13, my_event_cb, LV_EVENT_CLICKED, &start_x);

    // Button 1,4
    start_x = 30 + 390 + 180;
    lv_obj_t * btn14 = lv_btn_create(lv_scr_act());
    lv_obj_set_x(btn14, start_x);
    lv_obj_set_y(btn14, start_y);
    lv_obj_set_size(btn14, 180, 150);
    lv_obj_add_flag(btn14, LV_OBJ_FLAG_CHECKABLE);
    label = lv_label_create(btn14);
    lv_label_set_text(label, btn_name[4]);
    lv_obj_add_event_cb(btn14, my_event_cb, LV_EVENT_CLICKED, &start_x);

    // LV_IMG_DECLARE(imgbtn_left);
    // LV_IMG_DECLARE(imgbtn_right);
    // LV_IMG_DECLARE(imgbtn_mid);
    // lv_obj_t * btn88 = lv_btn_create(lv_scr_act());
    // //lv_imgbtn_set_src(btn88, LV_IMGBTN_STATE_..., src_left, src_center, src_right)
    // lv_img_set_src(btn88, &img_cogwheel_argb);
    // lv_imgbtn_set_src(btn88, LV_IMGBTN_STATE_RELEASED, &imgbtn_left, &imgbtn_mid, &imgbtn_right);


    // BUTTON 98
    //lv_obj_t * label;

    //lv_obj_t * btn98 = lv_btn_create(lv_scr_act());
    //lv_obj_add_event_cb(btn98, event_handler, LV_EVENT_ALL, NULL);
    // lv_obj_align(btn98, LV_ALIGN_CENTER, 200, -40);
    // label = lv_label_create(btn98);
    // lv_label_set_text(label, "Button");
    // lv_obj_center(label);

    // lv_obj_t * btn99 = lv_btn_create(lv_scr_act());
    // lv_obj_add_event_cb(btn99, event_handler, LV_EVENT_ALL, NULL);
    // lv_obj_align(btn99, LV_ALIGN_CENTER, 200, 40);
    // lv_obj_add_flag(btn99, LV_OBJ_FLAG_CHECKABLE);
    // lv_obj_set_height(btn99, LV_SIZE_CONTENT);
    // label = lv_label_create(btn99);
    // lv_label_set_text(label, "Toggle");
    // lv_obj_center(label);

    lv_ex_img_1();
    //lv_example_imgbtn_1();

    Serial.println("Setup done");
  }
}

void loop()
{
  lv_timer_handler(); /* let the GUI do its work */
  delay(5);
}
