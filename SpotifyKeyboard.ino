#include "Adafruit_TinyUSB.h"
#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>


//size of message
uint8_t msg[8];
uint8_t previous_msg[8];
#define msgsize 8

//pin define
#define like_btn D1
#define recomm_btn D2
#define playlist_btn D3
#define prev_btn D10
#define pause_btn D9
#define next_btn D8
#define volume_pot A0

int like_state;
int recomm_state;
int playlist_state;
int prev_state;
int pause_state;
int next_state;
int states[6] = {like_state, recomm_state, playlist_state, prev_state, pause_state, next_state};


int volume;
int filtered_reading;
unsigned long time_now=0;
unsigned long time_now1=0;
unsigned long previousMillis=0;

// Generic In Out with 16 bytes report (max)
uint8_t const desc_hid_report[] =
{
  TUD_HID_REPORT_DESC_GENERIC_INOUT(8)
};


// USB HID object and display. 
Adafruit_USBD_HID usb_hid(desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_NONE, 2, true);
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0);

// the setup function runs once when you press reset or power the board
void setup()
{

//setting type of pin
pinMode(like_btn, INPUT_PULLDOWN);  
pinMode(recomm_btn, INPUT_PULLDOWN);
pinMode(playlist_btn, INPUT_PULLDOWN);
pinMode(prev_btn, INPUT_PULLDOWN);
pinMode(pause_btn, INPUT_PULLDOWN);
pinMode(next_btn, INPUT_PULLDOWN);
pinMode(volume, INPUT);


attachInterrupt(digitalPinToInterrupt(like_btn), like, CHANGE);
attachInterrupt(digitalPinToInterrupt(recomm_btn), recommend, CHANGE);
attachInterrupt(digitalPinToInterrupt(playlist_btn), playlist, CHANGE);
attachInterrupt(digitalPinToInterrupt(prev_btn), previous, CHANGE);
attachInterrupt(digitalPinToInterrupt(pause_btn), pause, CHANGE);
attachInterrupt(digitalPinToInterrupt(next_btn), next, CHANGE);

#if defined(ARDUINO_ARCH_MBED) && defined(ARDUINO_ARCH_RP2040)
  TinyUSB_Device_Init(0);
#endif
//name of descriptor
  usb_hid.setStringDescriptor("Spotify Keyboard");

  usb_hid.setReportCallback(get_report_callback, set_report_callback);
  usb_hid.begin();
  u8g2.begin();
  Serial.begin(115200);

  // wait until device mounted
  while( !TinyUSBDevice.mounted() ) delay(1);

//clears message
for (int i=0; i<msgsize;i++){
msg[i]=0;
  }
}


void loop()
{
  u8g2.clearBuffer();					// clear the internal memory
  u8g2.setFont(u8g2_font_logisoso28_tr);  // choose a suitable font at https://github.com/olikraus/u8g2/wiki/fntlistall
  u8g2.drawStr(8,29,"SPOTIFY");	// write something to the internal memory
  u8g2.sendBuffer();
if (millis()<=300){
  previousMillis=0;
  time_now=0;
  time_now1=0;
}  	
//reads potentiometer
if(millis() - time_now >= 20){
  time_now=millis();
  previous_msg[6]=msg[6];
  int raw_reading=analogRead(volume_pot);
//low-pass filter 
  float  filter_constant=0.5;
  filtered_reading = filtered_reading * (1 - filter_constant) + raw_reading * filter_constant;
  volume = filtered_reading;  
  msg[6]=100-((volume-1)*100/1015);
  }
if (msg[6]!=previous_msg[6]){
previous_msg[7]=msg[7];
msg[7]=1;
}
else if (msg[6]==previous_msg[6]){
previous_msg[7]=msg[7];
msg[7]=0;
}





//checks value of buttons and sends report, sending report also has 100ms cooldown
//like
for (int i=0; i<=5;i++){
  previous_msg[i]=msg[i];
  msg[i]=states[i];
 }



//sends report
if ((millis()-time_now1>=50)&&(msg[0]!=previous_msg[0])||(msg[1]!=previous_msg[1])||(msg[2]!=previous_msg[2])||(msg[3]!=previous_msg[3])||(msg[4]!=previous_msg[4])||(msg[5]!=previous_msg[5])||(msg[7]!=previous_msg[7])){
  time_now1=millis();
  usb_hid.sendReport(0, msg, msgsize);
  
  }

}
void like(){
  states[0]=digitalRead(like_btn);
}
void recommend(){
  states[1]=digitalRead(recomm_btn);
}
void playlist(){
  states[2]=digitalRead(playlist_btn);  
}
void previous(){
  states[3]=digitalRead(prev_btn);  
}
void pause(){
  states[4]=digitalRead(pause_btn);
}
void next(){
  states[5]=digitalRead(next_btn);  
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t get_report_callback (uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  //not used
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;
  return 0;
}


// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void set_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  // This example doesn't use multiple report and report ID
  (void) report_id;
  (void) report_type;


  for (int i=0; i<bufsize;i++){
  Serial.print(buffer[i]); 
  Serial.print(" ");
 }
  // echo back anything we received from host
  usb_hid.sendReport(0, msg, msgsize);
}

