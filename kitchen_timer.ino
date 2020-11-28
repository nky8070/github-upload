/*
 An example digital clock using a TFT LCD screen to show the time.
 Demonstrates use of the font printing routines. (Time updates but date does not.)

 It uses the time of compile/upload to set the time
 For a more accurate clock, it would be better to use the RTClib library.
 But this is just a demo...

 Make sure all the display driver and pin comnenctions are correct by
 editting the User_Setup.h file in the TFT_eSPI library folder.

 #########################################################################
 ###### DON'T FORGET TO UPDATE THE User_Setup.h FILE IN THE LIBRARY ######
 #########################################################################

 Based on clock sketch by Gilchrist 6/2/2014 1.0

A few colour codes:

code  color
0x0000  Black
0xFFFF  White
0xBDF7  Light Gray
0x7BEF  Dark Gray
0xF800  Red
0xFFE0  Yellow
0xFBE0  Orange
0x79E0  Brown
0x7E0 Green
0x7FF Cyan
0x1F  Blue
0xF81F  Pink

 */
#define M5STACK_MPU6886 
#include <M5Stack.h>
#include <Adafruit_NeoPixel.h>
#define PIN            15
#define NUMPIXELS      10
#define TFT_GREY 0x5AEB
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

uint32_t targetTime = 0;                    // for next 1 second timeout
uint32_t finalTargetTime= 0;
uint32_t settingSeconds = 90;
static uint8_t conv2d(const char* p); // Forward declaration needed for IDE 1.6.x

uint8_t hh = conv2d(__TIME__), mm = conv2d(__TIME__ + 3), ss = conv2d(__TIME__ + 6); // Get H, M, S from compile time

byte omm = 99, oss = 99;
byte xcolon = 0, xsecs = 0;
unsigned int colour = 0;
unsigned int state = 0; //0:Stop 1:Running

void drawButtonLabel(){
  M5.Lcd.drawString("start/stop", 10, 200, 4);
  M5.Lcd.drawString("up", 140, 200, 4);
  M5.Lcd.drawString("down", 220, 200, 4);
}
void setup(void) {
  //Serial.begin(115200);
  M5.begin();
  M5.Power.begin();
  M5.IMU.Init();
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(TFT_BLACK);

  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(TFT_YELLOW, TFT_BLACK);

  finalTargetTime= millis()+ (settingSeconds * 1000);
  targetTime = millis() + 100;
  Serial.printf("final:%d, target: %d \n", finalTargetTime, targetTime);

  drawButtonLabel();
}

void showLed(int led){
  int col= led? 200: 0;
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(col, col, col)); 
    pixels.show(); 
  }  
}

int getUpside(){
  float accX, accY, accZ;
  M5.IMU.getAccelData(&accX,&accY,&accZ);
  Serial.printf("accY:%5.1f\n", accY);
  if(accY >= 1.0){
    return 1;
  }else if(accY <= -1.0){
    return 3;
  }else if(accX >= 1.0){
    return 2;
  }else if(accX <= -1.0){
    return 4;
  }else{
    return 0;
  }
}

void drawTime(){
  // Update digital time
  int xpos = 0;
  int ypos = 85; // Top left corner ot clock text, about half way down
  int ysecs = ypos + 24;
   if (omm != mm) { // Redraw hours and minutes time every minute
    omm = mm;
    // Draw hours and minutes
    if (hh < 10) xpos += M5.Lcd.drawChar('0', xpos, ypos, 8); // Add hours leading zero for 24 hr clock
    xpos += M5.Lcd.drawNumber(hh, xpos, ypos, 8);             // Draw hours
    xcolon = xpos; // Save colon coord for later to flash on/off later
    xpos += M5.Lcd.drawChar(':', xpos, ypos - 8, 8);
    if (mm < 10) xpos += M5.Lcd.drawChar('0', xpos, ypos, 8); // Add minutes leading zero
    xpos += M5.Lcd.drawNumber(mm, xpos, ypos, 8);             // Draw minutes
    xsecs = xpos; // Sae seconds 'x' position for later display updates
  }
  if (oss != ss) { // Redraw seconds time every second
    oss = ss;
    xpos = xsecs;
     if (mm % 2) { // Flash the colons on/off
      M5.Lcd.setTextColor(0x39C4, TFT_BLACK);        // Set colour to grey to dim colon
      M5.Lcd.drawChar(':', xcolon, ypos - 8, 8);     // Hour:minute colon
      xpos += M5.Lcd.drawChar(':', xsecs, ysecs, 6); // Seconds colon
      M5.Lcd.setTextColor(TFT_YELLOW, TFT_BLACK);    // Set colour back to yellow
      showLed(1);
    }
    else {
      M5.Lcd.drawChar(':', xcolon, ypos - 8, 8);     // Hour:minute colon
      xpos += M5.Lcd.drawChar(':', xsecs, ysecs, 6); // Seconds colon
      showLed(0);
    }
     //Draw seconds
    if (ss < 10) xpos += M5.Lcd.drawChar('0', xpos, ysecs, 6); // Add leading zero
    M5.Lcd.drawNumber(ss, xpos, ysecs, 6);                     // Draw seconds
  }
}
void loop() {
  hh= 1;
  mm= 30;
  ss= 00;
  if(state){
    //Runing
    if(finalTargetTime < millis()){
      hh=0;
      mm=0;
      ss=0;
      drawTime();
      Serial.println("End"); 
      while(1){
        M5.update();
        if(M5.BtnA.wasReleased()|| getUpside() == 1){
          M5.Lcd.setRotation(1);
          M5.Lcd.clear();
          drawButtonLabel();
          state= 0; //Waiting
          break;
        }else{
          //M5.Speaker.tone(881, 5);
          M5.Speaker.tone(990, 1);
          //M5.Speaker.tone(1762, 5);
          delay(120);
        }
      }
    }else{
      if(targetTime < millis()){
        uint32_t rest = finalTargetTime - targetTime;
        hh= rest / (60 * 1000);
        mm= (rest % (60 * 1000))/ 1000;
        ss= (rest % 1000)/ 10;
        Serial.printf("%d:%d:%d\n",hh, mm, ss);
        // Set next update for 1 second later
        targetTime = millis() + 10;
        drawTime();
      }
   }
  }else{
    //Waiting
    M5.update();
    if(M5.Power.canControl()){
      M5.Lcd.drawString("batt:", 200, 10, 4);
      M5.Lcd.drawNumber(M5.Power.getBatteryLevel(), 260, 10, 4);
    }
    M5.Lcd.drawNumber(getUpside(), 10, 10, 4);
    
    if(M5.BtnA.wasReleased()|| getUpside() == 3){
      if(getUpside()== 3){
        M5.Lcd.setRotation(3);
      }
      M5.Lcd.clear();
      //Start Timer
      state= 1; //Running
      finalTargetTime= millis()+ (settingSeconds * 1000);
      targetTime = millis() + 10;
    }
    if(M5.BtnB.wasReleased()){
      //Count Up
      settingSeconds += 10;
    }
    if(M5.BtnC.wasReleased()){
      //Count Down
      settingSeconds -= 10;
    }
    hh= settingSeconds / 60;
    mm= settingSeconds % 60;
    ss= 0;
    oss=-1;//強制描画
    drawTime();
    delay(100);
  }
}

// Function to extract numbers from compile time string
static uint8_t conv2d(const char* p) {
  uint8_t v = 0;
  if ('0' <= *p && *p <= '9')
    v = *p - '0';
  return 10 * v + *++p - '0';
}
