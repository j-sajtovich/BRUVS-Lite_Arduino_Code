/* BRUVS controller code.

   Developers: Frank Landry, Jack Tsao
   Date: July 16, 2021
   Last updated: July 28, 2022 (by Dave Burns)

   This code is to be used to control the 2 BRUVS cameras in a Primary/Secondary configuration.
   The code will run on both primary and secondary cameras, which is currently hard set. Camera
   modes can be set to either video or photo capture. Lights can be set to dynamic or
   static. Once the record time and intervals are set, the primary camera will begin the
   handshake sequence (syncing), turn on the camera on each system, start recording, turn on
   the lights, turn off the lights, stop recording, and then wait the set interval time before
   it begins again.

   NOTE: To set each driver board as either Primary or Secondary, set the position jumper pin
         located on the driver board.

         The yellow indicator light located on the Arduino microcontroller board will turn
         on when waiting to synchronize with it's paired Arduino microcontroller and turn
         off when synchronization is complete.
         
         See change.log file for updated information.

*/

/*=========================================================================================*
                                    Libraries
  =========================================================================================*/
#include <OneWire.h>
#include <DallasTemperature.h>


/*=========================================================================================*
                                    Pin definitions
  =========================================================================================*/
// ---WARNING!--- Do not change values!

#define MstrOUT_SlvIN         A0
#define LDR                   A2
#define MstrIN_SlvOUT         A3
#define MstrTimeOUT           A4
#define Temp_sensor           A5
#define Batt_Voltage          A7
#define CameraEnable          2       // HIGH to turn on
#define Photo                 4
#define Video                 5
#define SystemOff             7       // HIGH to turn off everything
#define Fan                   9
#define LED_Driver_PWM        10
#define LED_Driver_On         11
#define Mstr_Slv              12


/*==========================================================================================*
                                   Global variable definition
  ==========================================================================================*/
// ---WARNING!--- Do not change settings unless absolutely neccessary!

bool Mstr = false;                 // True if camera is primary
bool FanStatus = false;               // False = off, true = on
bool IntervalCheck = false;
static int CaptureMode = 0;
static int MaxTempSetPt = 80;         // Maximum ambient case temperature. 125C on regulator
static int FanTemp = 40;              // Temperature that triggers fan
static int LDRValue = 0;
unsigned long startMillis;
unsigned long currentMillis;
unsigned long previousMillis;
float temperature;
OneWire oneWire(Temp_sensor);
DallasTemperature sensors(&oneWire);

void BatteryCheck();
void CaptureStartStop();
void FanON();
void FanOFF();
void Sync();
void LightON(int);
void LightOFF();
int getLDR();
/*==========================================================================================*
                                   Mode selection
  ==========================================================================================*/
bool VideoMode = true;                // True = video, false = photo
/*bool DynamicLights = false;         // True = will use LDR to calibrate lights, false = disable
                                         Currently not implemented */


/*==========================================================================================*
                                   Settings
  ==========================================================================================*/
float BattCutoffVolt = 13.8;          // Minimum battery capacity. 13.8V ~30% total capacity.
static int RecordTime = 30000;        // Time in milliseconds(ex: 1sec = 1000)
static int LightLevel = 200;          // [0-255] 0=full on, 255=off
unsigned long Interval = 30000;      // 5 minutes =300000


/*==========================================================================================*
                                   Initializing Camera System
  ==========================================================================================*/
void setup() {
  sensors.begin();
 
  pinMode(LED_BUILTIN, OUTPUT);       // Adruino LED on while while waiting to sync
  digitalWrite(LED_BUILTIN, LOW);     //just added this
  
  pinMode(Mstr_Slv, INPUT);
      
  if (digitalRead(Mstr_Slv) == HIGH) {
    Mstr = true; 
    pinMode(MstrTimeOUT, OUTPUT);     
    digitalWrite(MstrTimeOUT, LOW);
    pinMode(MstrIN_SlvOUT, INPUT);
    pinMode(MstrOUT_SlvIN, OUTPUT);
    digitalWrite(MstrOUT_SlvIN, LOW);
  }
  
  else if (digitalRead(Mstr_Slv) == LOW) {  
    pinMode(MstrTimeOUT, INPUT);
    pinMode(MstrOUT_SlvIN, INPUT);
    pinMode(MstrIN_SlvOUT, OUTPUT);
    digitalWrite(MstrIN_SlvOUT, LOW);
  }
    
  pinMode(Photo, OUTPUT);
  digitalWrite(Photo, LOW);
  pinMode(Video, OUTPUT);
  digitalWrite(Video, LOW);
  pinMode(CameraEnable, OUTPUT);
  digitalWrite(CameraEnable, LOW);
  pinMode(SystemOff, OUTPUT);
  digitalWrite(SystemOff, LOW);
  pinMode(LED_Driver_PWM, OUTPUT);
  analogWrite(LED_Driver_PWM, 255);
  pinMode(LED_Driver_On, OUTPUT);
  digitalWrite(LED_Driver_On, LOW);
  pinMode(Fan, OUTPUT);
  digitalWrite(Fan, LOW);
  pinMode(Batt_Voltage, INPUT);
  startMillis = millis();
  previousMillis = startMillis;
  
//  if (VideoMode == true)
//    CaptureMode = 5;
//  else if (VideoMode == false)
//    CaptureMode = 4;
}



/*==========================================================================================*
                                   Main
  ==========================================================================================*/
void loop() {
  BatteryCheck();                     // Check battery voltage level. If low, system will shutdown.
 
  if (Mstr == true) {                      // Check current time if the camera is the primary camera
    currentMillis = millis();         // Check if currently within Interval time.
    if ((currentMillis - previousMillis) >= Interval) { 
      digitalWrite(MstrTimeOUT, HIGH);
      IntervalCheck = true;
      previousMillis = currentMillis;
    }
  }
  
  else if (Mstr == false){
    if (digitalRead(MstrTimeOUT) == HIGH){ // Slave check if within Interval time. 
      IntervalCheck = true;
    }
  }
  
  sensors.requestTemperatures();      // Check driver board temp
  temperature = sensors.getTempCByIndex(0);
  if (temperature >= FanTemp)
    FanON();
  else
    FanOFF();
  if (temperature < MaxTempSetPt) {   // If temp is within range
    //Sync();                           // Start recording sequence: Sync with other arduino    
    if (IntervalCheck == true) {              // Turn on camera if within Interval time
      Sync();
      digitalWrite(MstrTimeOUT, LOW); 
      digitalWrite(CameraEnable, HIGH);
      delay(10000);                  // Wait 5 seconds before recording session starts
      FanON();
      //if (VideoMode == true) {                // Check mode(pic/vid) and run sequence
      CaptureStartStop();           // Start record    
     // digitalWrite(5, HIGH);
      delay(1000);                  // 1 sec delay
      LightON(LightLevel);          // Turn on lights
      delay(RecordTime);            // Record video for "RecondTime" length of time
      LightOFF();                   // Turn off lights
      delay(1000);
     //digitalWrite(5, LOW);
      CaptureStartStop();           // Stop recording
      //}       
//      else {                          // Only runs in picture mode
//        LightON(LightLevel);
//        CaptureStartStop();
//        LightOFF();
//      }
      delay(5000);   //was 1000
      digitalWrite(CameraEnable, LOW);
      //digitalWrite(MstrTimeOUT, LOW);
      IntervalCheck = false;          // Reset interval check
      previousMillis = millis();
    }
  }
  
  else {                              // If temp has reached max temp
    if(FanStatus == false)            // check if fan is on
      FanON();                        // turn on fan if it is off and skip recording interval
    IntervalCheck = false;            // Reset interval check
    if (Mstr == true)
      digitalWrite(MstrTimeOUT, LOW);
  }
}

/*==========================================================================================*
                                   Functions
  ==========================================================================================*/
void CaptureStartStop() {
//  digitalWrite(CaptureMode, HIGH);
//  delay(1000);
//  digitalWrite(CaptureMode, LOW);

  digitalWrite(5, HIGH);
  delay(1000);
  digitalWrite(5, LOW);
}

void Sync() {
  if (Mstr == true) {
    digitalWrite(LED_BUILTIN, HIGH);  // Turn on Sync Light
    digitalWrite(MstrOUT_SlvIN, HIGH);
    while (digitalRead(MstrIN_SlvOUT) == LOW);
    delay(50);
    digitalWrite(MstrOUT_SlvIN, LOW);
    digitalWrite(LED_BUILTIN, LOW);   // Turn off Sync Light
  }
  else if (Mstr == false){
    digitalWrite(LED_BUILTIN, HIGH);  // Turn on Sync Light
    digitalWrite(MstrIN_SlvOUT, HIGH);
    while (digitalRead (MstrOUT_SlvIN) == LOW);
    delay(50);
    digitalWrite(MstrIN_SlvOUT, LOW);
    digitalWrite(LED_BUILTIN, LOW);   // Turn off Sync Light
  }
  return;
}

void LightON(int k) {
  digitalWrite(LED_Driver_On, HIGH);
  delay(100);
  for (int i = 255; i >= k; i -= 5)
  {
    analogWrite(LED_Driver_PWM, i);
    delay(100);
  }
}

void LightOFF() {
  analogWrite(LED_Driver_PWM, 255);
  digitalWrite(LED_Driver_On, LOW);
}

void FanON() {
  digitalWrite(Fan, HIGH);
  FanStatus = true;
}

void FanOFF() {
  digitalWrite(Fan, LOW);
  FanStatus = false;
}

void BatteryCheck() {
  int value;
  value = analogRead(Batt_Voltage);
  if (value <= 565)
    digitalWrite(SystemOff, HIGH);
}

int getLDR() {
  int value;
  value = analogRead(LDR);
  delay(100);
  return value;
}
