//* SUNTON_8048_v50: 20/08/23
//* - Meters are now working
//* - Buttons are working - a bit slow
//* SUNTON_8048_v51: 20/08/23
//* - Backlight on/off with home Button - Button1
//* - Added float for varTemp meter display
//* SUNTON_8048_v52: 20/08/23
//* - Clean-up unwnted files
//* - Upgrade lvgl to v8.3.9

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <Arduino_GFX.h>
//#include <../lvgl/lvgl.h>
#include <lvgl.h>
#include <TAMC_GT911.h>
#include "WiFi.h"

#include <iostream>
#include <string>

//#include <Arduino_JSON.h>

#include "painlessMesh.h"

#define LV_USE_USER_DATA      1

/*Garbage Collector settings
 *Used if lvgl is binded to higher level language and the memory is managed by that language*/
#define LV_ENABLE_GC 0
#if LV_ENABLE_GC != 0
#  define LV_GC_INCLUDE "gc.h"                           /*Include Garbage Collector related things*/
#endif /*LV_ENABLE_GC*/

/*===================PainlessMesh====================*/
#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555
//  ######################## painlessMESH ################################

int start_x = 30;
int start_y = 10;

int button0_state = 0;   //OFF
int button1_state = 0;   //OFF
int button2_state = 0;
int button3_state = 0;
int button4_state = 0;
int button5_state = 0;  
int button6_state = 0;
int button7_state = 0;
int button8_state = 0;

int button_state = 0;

float varTemp = 0.0;
int intTemp  = 0;
float varHumid = 0;
int intHumid = 0;

static uint32_t buttonx = 0;

char *btn_name[] = {"ZERO - DONT USE", "Button11", "Button12", "Button13",
                     "Button14", "Button15", "Button16"};

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

bool calc_delay = false;
SimpleList<uint32_t> nodes;

// Prototype so PlatformIO doesn't complain
void lv_temperature_meter(float varTemp, int intTemp); 
void lv_humidity_meter(int intHumid);
void sendMessage() ; 
void set_value(void);

Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

void sendMessage() {
  String msg = "";
  //Serial.printf("Start Sending....");
  DynamicJsonDocument doc(1024);

  doc["Node"] = "Sunton_Screen";
  doc["Button"] = buttonx;
  doc["Status"] = button_state;

  serializeJson(doc, msg);

  //msg += mesh.getNodeId();
  mesh.sendBroadcast( msg );
  //taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 5 ));
  taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 2 ));

  buttonx = 99;
  button_state = 99;
}

//Not sure why but Serial.printf does not output to the terminal
void print_something(char* varPrint){
  printf(varPrint);
}

//varTemp = 0;
//varHumid = 0;

char buffer[100];

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  //Serial.printf("8-Port: Received from %u msg=%s\n", from, msg.c_str());
  //sprintf(buffer, "startHere: Received from %u msg=%s\n", from, msg.c_str());
  //print_something(buffer);

  if (from == 2666647481){    // DHT   (SUNTON SCREEN - 4209014484)
    //##################################################################################

    String json;
    DynamicJsonDocument doc(1024) ;
    //json = msg.c_str();
    json = msg;

    DeserializationError error = deserializeJson(doc, json);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      //return;
    }

    // Fetch values.
    const char* node = doc["Node"];

    varTemp = doc["Temp"];                   //Float
    //sprintf(buffer, "JSON: Received from %u Temp=%d\n", from, varTemp);
    //print_something(buffer);
    intTemp = static_cast<int>(varTemp);   // Convert Float to Int

    lv_temperature_meter(varTemp, intTemp);  //Redraw the temperature meter

    varHumid = doc["Humid"];
    intHumid = static_cast<int>(varHumid);

    lv_humidity_meter(intHumid);  //Redraw the temperature meter

    sprintf(buffer, "startHere: Received from %u msg=%s\n", from, msg.c_str());
    print_something(buffer);
    //#################################################################################
  }

  if (from == 329672885){ //GEEKWORM - RED 
    // Do Something
  }
  //ELSE
  //Serial.println(from);
  //Serial.printf("8-Port: Received from %u msg=%s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
  //Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
  sprintf(buffer, "--> startHere: New Connection, nodeId = %u\n", nodeId);
  print_something(buffer);
}
void changedConnectionCallback() {
  //Serial.printf("Changed connections\n");
  sprintf(buffer, "Changed connections\n");
  print_something(buffer);
}
void nodeTimeAdjustedCallback(int32_t offset) {
  //Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
  sprintf(buffer, "Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
  print_something(buffer);
}
//########################### PainlessMESH - Finish #####################################

/*===================TouchConfig====================*/
#define TOUCH_SDA 19
#define TOUCH_SCL 20
#define TOUCH_INT 18
#define TOUCH_RST 38

#define TOUCH_MAP_X1 800
#define TOUCH_MAP_X2 0
#define TOUCH_MAP_Y1 480
#define TOUCH_MAP_Y2 0

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
    0 /* pclk_active_neg */, 13000000 /* prefer_speed */);
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

LV_IMG_DECLARE(img_cogwheel_argb);
LV_IMG_DECLARE(cogwheel);
LV_IMG_DECLARE(cogwheel2);
LV_IMG_DECLARE(img_home_icon_80);

char button_14 = 'a';
static void my_event_cb(lv_event_t * event)
{
    static uint32_t cnt = 1;
    lv_obj_t * btn = lv_event_get_target(event);
    lv_obj_t * label = lv_obj_get_child(btn, 0);

    cnt++;
}

//##########  Do Someting if Button Pressed  ##########
void Button_Touched(int button){
    printf ("Button Pressed: %2d \n", button);
    if(button == 1){
    //printf( "1 Clicked!: \n");
    printf ("Button 1 State - before: %2d \n", button3_state);
    //printf ("Preceding with blanks: %2d \n", button);
    if (button1_state == 0){
        button1_state = 1;
        digitalWrite(GFX_BL, HIGH);
      } else{
        button1_state = 0;
        digitalWrite(GFX_BL, LOW);
    }
      button_state = button1_state;
      //printf ("Button 1 State - after: %2d \n", button3_state);
    }
    if(button == 2){
    printf ("Button 2 State - before: %2d \n", button3_state);
    if (button2_state == 0){
        button2_state = 1;
      } else{
        button2_state = 0;
    }
    button_state = button2_state;
    }
    if(button == 3){
      if (button3_state == 0){
          button3_state = 1;
        } else{
          button3_state = 0;
      }
      button_state = button3_state;
    }
    if(button == 4){
      if (button4_state == 0){
          button4_state = 1;
        } else{
          button4_state = 0;
      }
      button_state = button4_state;
    }
      if(button == 5){
      if (button5_state == 0){
          button5_state = 1;
        } else{
          button5_state = 0;
      }
      button_state = button5_state;
    }
      if(button == 6){
      if (button6_state == 0){
          button6_state = 1;
        } else{
          button6_state = 0;
      }
      button_state = button6_state;
    }
      if(button == 7){
      if (button7_state == 0){
          button7_state = 1;
        } else{
          button7_state = 0;
      }
      button_state = button7_state;
    }
      if(button == 8){
      if (button8_state == 0){
          button8_state = 1;
        } else{
          button8_state = 0;
      }
      button_state = button8_state;
    }

}
//##########################################################################

#define NUM_BUTTONS		8

static lv_obj_t		*buttons[NUM_BUTTONS];

static void btn_cb( lv_event_t *event ) {
	lv_obj_t *btn = lv_event_get_target(event);
	lv_obj_t *label = lv_obj_get_child(btn, 0);  // Get a pointer to the label object containing button text
	
  if( btn == buttons[0] ) {            // Compare the object pointers to discover which object was clicked
      //printf( "1 Clicked!\n");
      buttonx = 1;
      Button_Touched(buttonx);
      //lv_label_set_text_fmt(label,"%" LV_PRIu32, btn);
    } else if( btn == buttons[1] ) {
        buttonx = 2;
        Button_Touched(buttonx);
    } else if( btn == buttons[2] ) {
        buttonx = 3;
        Button_Touched(buttonx);
    } else if( btn == buttons[3] ) {
        buttonx = 4;
        Button_Touched(buttonx);
    } else if( btn == buttons[4] ) {
        buttonx = 5;
        Button_Touched(buttonx);
    } else if( btn == buttons[5] ) {
        buttonx = 6;
        Button_Touched(buttonx);
    } else if( btn == buttons[6] ) {
        buttonx = 7;
        Button_Touched(buttonx);
    } else if( btn == buttons[7] ) {
        buttonx = 8;
        Button_Touched(buttonx);
	  }
	//printf( "Button with label %s Clicked!\n", lv_label_get_text(label));    // Get the label text
}

static void create_buttons1( lv_obj_t *parent ) {

	lv_obj_t	*btn_txt;
  //lv_obj_t * scr1  = lv_obj_create(NULL);

  for( uint8_t i = 0; i < NUM_BUTTONS; i++ ) {
		buttons[i] = lv_btn_create( parent );
		btn_txt = lv_label_create( buttons[i] );
    lv_obj_set_size(buttons[i], 180, 150);
		lv_label_set_text_fmt( btn_txt, "Button %d", i+1  );
    lv_obj_add_flag(buttons[i], LV_OBJ_FLAG_CHECKABLE);
		lv_obj_set_pos( buttons[i], 16 + (196 * i), 14 );
    //lv_obj_set_pos(obj, x, y)
		lv_obj_add_event_cb(buttons[i], btn_cb, LV_EVENT_CLICKED, NULL);
	}
  start_x = 65;  // Across
  start_y = 60;  // Down
  lv_obj_t * img1 = lv_img_create(lv_scr_act());
  //lv_img_set_src(img1, &img_cogwheel_argb);
  lv_img_set_src(img1, &img_home_icon_80);
  lv_obj_align(img1, LV_ALIGN_TOP_LEFT, start_x, start_y);
  //lv_obj_set_pos( img1, (20 + (190 * i), 20 );
  //lv_obj_set_x(img1, start_x);
  //lv_obj_set_y(img1, start_y);
}

//int xyz = 0;
static lv_obj_t * meter;


void lv_temperature_meter(float varTemp, int intTemp)
{
    
    //sprintf(buffer, "lv_temperature_meter: varTemp=%.1f\n", varTemp);
    //print_something(buffer);
    //sprintf(buffer, "lv_temperature_meter: intTemp=%d\n", intTemp);
    //print_something(buffer);

    //int intTemp = random(0, 50);

    meter = lv_meter_create(lv_scr_act());
    //lv_obj_center(meter);
    lv_obj_set_x(meter, 30);
    lv_obj_set_y(meter, 200);
    lv_obj_set_size(meter, 230, 230);

    /*Add a scale first*/
    lv_meter_scale_t * scale = lv_meter_add_scale(meter);

    lv_meter_set_scale_range(meter, scale, 0, 50, 250, 145);  //0, 50, 250, 145
    // MIN
    // MAX
    // Angle Range in degrees
    // Start - offset from 3 o'clock

    //lv_meter_set_scale_ticks(meter, scale, 41, 2, 10, lv_palette_main(LV_PALETTE_GREY));
    lv_meter_set_scale_ticks(meter, scale, 51, 2, 10, lv_palette_main(LV_PALETTE_GREY));
    //lv_meter_set_scale_major_ticks(meter, scale, 8, 4, 15, lv_color_black(), 10);
    lv_meter_set_scale_major_ticks(meter, scale, 10, 4, 15, lv_color_black(), 10);


    /*Add a blue arc to the start*/
    lv_meter_indicator_t * indic;
    indic = lv_meter_add_arc(meter, scale, 3, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_meter_set_indicator_start_value(meter, indic, 0);
    lv_meter_set_indicator_end_value(meter, indic, 10);

    /*Make the tick lines blue at the start of the scale*/
    indic = lv_meter_add_scale_lines(meter, scale, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_BLUE), false, 0);
    lv_meter_set_indicator_start_value(meter, indic, 0);
    lv_meter_set_indicator_end_value(meter, indic, 10);

    /*Add a red arc to the end*/
    indic = lv_meter_add_arc(meter, scale, 3, lv_palette_main(LV_PALETTE_RED), 0);
    lv_meter_set_indicator_start_value(meter, indic, 35);
    lv_meter_set_indicator_end_value(meter, indic, 50);

    /*Make the tick lines red at the end of the scale*/
    indic = lv_meter_add_scale_lines(meter, scale, lv_palette_main(LV_PALETTE_RED), lv_palette_main(LV_PALETTE_RED), false, 0);
    lv_meter_set_indicator_start_value(meter, indic, 35);
    lv_meter_set_indicator_end_value(meter, indic, 50);

    /*Add a needle line indicator*/
    indic = lv_meter_add_needle_line(meter, scale, 4, lv_palette_main(LV_PALETTE_GREY), -10);

    // Set the value
    lv_meter_set_indicator_value(meter, indic, intTemp);

    /*Create label style*/
    static lv_style_t style;
    lv_style_init(&style);

    // Position the labels*/
    lv_style_set_text_font(&style, &lv_font_montserrat_22);  /*Set a larger font*/

    lv_obj_t * temp_label1 = lv_label_create(lv_scr_act());
    lv_obj_add_style(temp_label1 , &style, 0);
    lv_obj_align(temp_label1 , LV_ALIGN_LEFT_MID, 125, 110);   //across, down
    //lv_label_set_text_fmt( temp_label1, "%d", intTemp);
    lv_label_set_text_fmt( temp_label1, "%.1f", varTemp);    //you need to enable float in lv_conf.h
    
    String strTemp = "blah";
    //String pi = "pi is " + to_string(3.1415926);
    float val = 2.5;

    lv_obj_t * temp_label2 = lv_label_create(lv_scr_act());
    lv_obj_add_style(temp_label2 , &style, 0);
    lv_obj_align(temp_label2 , LV_ALIGN_LEFT_MID, 110, 165); //105 165
    lv_label_set_text(temp_label2 , "Temp\n");
}

void lv_humidity_meter(int intHumid)
{
    //sprintf(buffer, "lv_temperature_meter: Temp=%d\n", intHumid);
    //print_something(buffer);

    //int intHumid = random(0, 100);

    meter = lv_meter_create(lv_scr_act());
    lv_obj_set_x(meter, 30+250);
    lv_obj_set_y(meter, 200);
    lv_obj_set_size(meter, 230, 230);

    /*Add a scale first*/
    lv_meter_scale_t * scale = lv_meter_add_scale(meter);

    lv_meter_set_scale_range(meter, scale, 0, 100, 250, 145);
    // MIN
    // MAX
    // Angle Range in degrees
    // Start - offset from 3 o'clock

    lv_meter_set_scale_ticks(meter, scale, 41, 2, 10, lv_palette_main(LV_PALETTE_GREY));
    lv_meter_set_scale_major_ticks(meter, scale, 8, 4, 15, lv_color_black(), 10);

    /*Add a blue arc to the start*/
    lv_meter_indicator_t * indic;
    indic = lv_meter_add_arc(meter, scale, 3, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_meter_set_indicator_start_value(meter, indic, 0);
    lv_meter_set_indicator_end_value(meter, indic, 20);

    /*Make the tick lines blue at the start of the scale*/
    indic = lv_meter_add_scale_lines(meter, scale, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_BLUE), false, 0);
    lv_meter_set_indicator_start_value(meter, indic, 0);
    lv_meter_set_indicator_end_value(meter, indic, 20);

    /*Add a red arc to the end*/
    indic = lv_meter_add_arc(meter, scale, 3, lv_palette_main(LV_PALETTE_RED), 0);
    lv_meter_set_indicator_start_value(meter, indic, 80);
    lv_meter_set_indicator_end_value(meter, indic, 100);

    /*Make the tick lines red at the end of the scale*/
    indic = lv_meter_add_scale_lines(meter, scale, lv_palette_main(LV_PALETTE_RED), lv_palette_main(LV_PALETTE_RED), false, 0);
    lv_meter_set_indicator_start_value(meter, indic, 80);
    lv_meter_set_indicator_end_value(meter, indic, 100);

    /*Add a needle line indicator*/
    indic = lv_meter_add_needle_line(meter, scale, 4, lv_palette_main(LV_PALETTE_GREY), -10);

    // Set the value
    lv_meter_set_indicator_value(meter, indic, intHumid);

    /*Create label style*/
    static lv_style_t style;
    lv_style_init(&style);

    /*Position the labels*/
    lv_style_set_text_font(&style, &lv_font_montserrat_22);  /*Set a larger font*/

    lv_obj_t * temp_label1 = lv_label_create(lv_scr_act());
    lv_obj_add_style(temp_label1 , &style, 0);
    lv_obj_align(temp_label1 , LV_ALIGN_LEFT_MID, 130+250, 110);   //across, down
    lv_label_set_text_fmt( temp_label1, "%d", intHumid);

    lv_obj_t * temp_label2 = lv_label_create(lv_scr_act());
    lv_obj_add_style(temp_label2 , &style, 0);
    lv_obj_align(temp_label2 , LV_ALIGN_LEFT_MID, 100+250, 165);
    lv_label_set_text(temp_label2 , "Humid\n");
}

static void create_buttons2( lv_obj_t *parent ) {
  //* Create lvgl objs */
  lv_obj_t	*label;
  // ##################### BUTTONS #######################//
  // Button 1,1
  start_x = 30;  // Across
  start_y = 210;  // Down
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
}
//##########################################################################

void img_big_cogwheel(void)
{
    printf( "Drawing Image COGWHEEL!\n");
    start_x = 250;  // Across
    start_y = 50;  // Down
    lv_obj_t * img1 = lv_img_create(lv_scr_act());
    lv_img_set_src(img1, &img_cogwheel_argb);
    //lv_obj_align(img1, LV_ALIGN_CENTER, 400, 40);
    lv_obj_set_x(img1, start_x);
    lv_obj_set_y(img1, start_y);
}

void limg_little_cogwheel(void)
{
    // Little CogWheel
    lv_obj_t * img3 = lv_img_create(lv_scr_act());
    lv_img_set_src(img3, &cogwheel);
    lv_obj_set_x(img3, 20);
    lv_obj_set_y(img3, 400);
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

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


//static lv_obj_t * meter;

static void set_value_cb(void * indic, int32_t v)
{
    //lv_meter_set_indicator_value(meter, indic, v);
}

/**
 * A simple meter
 */
void lv_example_meter_1(void)
{
    meter = lv_meter_create(lv_scr_act());
    lv_obj_center(meter);
    lv_obj_set_size(meter, 200, 200);

    /*Add a scale first*/
    lv_meter_scale_t * scale = lv_meter_add_scale(meter);
    lv_meter_set_scale_ticks(meter, scale, 41, 2, 10, lv_palette_main(LV_PALETTE_GREY));
    lv_meter_set_scale_major_ticks(meter, scale, 8, 4, 15, lv_color_black(), 10);

    lv_meter_indicator_t * indic;

    /*Add a blue arc to the start*/
    indic = lv_meter_add_arc(meter, scale, 3, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_meter_set_indicator_start_value(meter, indic, 0);
    lv_meter_set_indicator_end_value(meter, indic, 20);

    /*Make the tick lines blue at the start of the scale*/
    indic = lv_meter_add_scale_lines(meter, scale, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_BLUE),
                                     false, 0);
    lv_meter_set_indicator_start_value(meter, indic, 0);
    lv_meter_set_indicator_end_value(meter, indic, 20);

    /*Add a red arc to the end*/
    indic = lv_meter_add_arc(meter, scale, 3, lv_palette_main(LV_PALETTE_RED), 0);
    lv_meter_set_indicator_start_value(meter, indic, 80);
    lv_meter_set_indicator_end_value(meter, indic, 100);

    /*Make the tick lines red at the end of the scale*/
    indic = lv_meter_add_scale_lines(meter, scale, lv_palette_main(LV_PALETTE_RED), lv_palette_main(LV_PALETTE_RED), false,
                                     0);
    lv_meter_set_indicator_start_value(meter, indic, 80);
    lv_meter_set_indicator_end_value(meter, indic, 100);

    /*Add a needle line indicator*/
    indic = lv_meter_add_needle_line(meter, scale, 4, lv_palette_main(LV_PALETTE_GREY), -10);

    /*Create an animation to set the value*/
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_exec_cb(&a, set_value_cb);
    lv_anim_set_var(&a, indic);
    lv_anim_set_values(&a, 0, 100);
    lv_anim_set_time(&a, 2000);
    lv_anim_set_repeat_delay(&a, 100);
    lv_anim_set_playback_time(&a, 500);
    lv_anim_set_playback_delay(&a, 100);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);
}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

void setup()
{
  Serial.begin(115200);
  Serial.println("TAMC_GT911 Example: Ready");

  //PainlessMESH
  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();

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

 //################  Display on Screen  ##########################

    // Little CogWheel
    //img_little_cogwheel();

    create_buttons1( lv_scr_act() );

    //create_buttons2( lv_scr_act() );
    // Big CogWheel
    img_big_cogwheel();

    varTemp = 1.1;
    intTemp = 0;

    lv_temperature_meter(varTemp, intTemp);
    lv_humidity_meter(intHumid);

    //lv_scr_load(scr1);
    //lv_scr_load(scr2);

    Serial.println("Setup done");
  }
}

void loop()
{
  lv_timer_handler(); /* let the GUI do its work */
  delay(5);
  mesh.update();
}
