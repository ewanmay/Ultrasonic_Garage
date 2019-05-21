/******************************************************************************************

   Ultrasonic Garage

*******************************************************************************************
  This application connects our garage door IOT device to Blynk

*/

//#define BLYNK_DEBUG // Optional, this enables lots of prints
//#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#include "credentials.h"
#ifndef PROJECT_TOKEN
#define PROJECT_TOKEN ""
#endif
#ifndef MY_SSID
#define MY_SSID ""
#endif
#ifndef PASS
#define PASS ""
#endif

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = PROJECT_TOKEN;

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = MY_SSID;
char pass[] = PASS;

/***********************************************************


   Blynk declarations


 ************************************************************/
// Attach virtual serial terminal to Virtual Pin V1
WidgetTerminal terminal(V1);

// set up a timer to help with several functions
BlynkTimer timer;


/************************************************

  Node MCU pin declarations

************************************************/

// Ultrasonic Sensor
const int trigPin = D3;
const int echoPin = D5;

// onboard LED
#define LED_ESP D4  //blue led onboard the ESP8266
#define LED LED_ESP
//#define LED LED_BUILTIN  //Red led onbaoard the NodeMCU dev board

// Outputs to Relay
const int doorPin = D0;
const int lightPin = D7;
byte switch_pins[] = { doorPin, lightPin }; // collective gpio to be used as switch/relay control

// Buzzer
//const int buzzerPin = D0;

// Photocell
const int photocellPin = A0;

// temperature
#define ONE_WIRE_BUS D6


/************************************************

  Supporting functions and declarations

************************************************/

bool connected_to_server = false; //boolean to indicate connection
bool heading_test_line; //boolean to indicate whether the heading of the test line has been output
bool update_terminal = false; //boolean indicates whether to dump garage status to terminal when updating blynk
int LED_timer; //reference to connected led timer
int RSSI_timer; //reference to RSSI measurement timer
int scan_status_timer; //reference to check status timer
int update_blynk_timer; //reference to update blynk timer
int test_timer; //reference to periodic test timer

#define LED_INTV_NO_CONNECT 500L  //interval (ms) between LED blinks when not connected
#define LED_INTV_CONNECT 5000L  //interval (ms) between LED blinks when connected
#define RSSI_INTV 2000L //interval (ms) between RSSI checks
#define  SCAN_INTV_FAST 200L //interval (ms) between status checks in fast mode
#define  SCAN_INTV_SLOW 255L //interval (ms) between status checks in slow mode
#define  UPDATE_INTV_FAST 1000L //interval (ms) between updates to blynk in fast mode
#define  UPDATE_INTV_SLOW 10000L //interval (ms) between updates to blynk in slow mode
#define  TEST_INTV_FAST 201L //interval (ms) between test loops in fast mode
#define  TEST_INTV_SLOW 256L //interval (ms) between test loops in slow mode

enum Door_Status { opened, opening, stuck, closing, closed, alarm };
enum Update_Speed { slow, fast };
String status_string[] = { "Open", "Opening", "Stuck", "Closing", "Closed", "Alarm" };

struct Light
{
  bool is_on;
  float level;
};

struct Garage_Status
{
  int door_position;
  long door_position_micros;
  float door_speed;
  Door_Status door_status;
  String door_status_string;
  Light light;
  float temperature;
  Update_Speed update_speed;
};

//Garage_Status garage{ 0,0,opened,"Open", {false, 999.999F}, 100.0F, fast };
Garage_Status garage;

void change_update_interval(Door_Status status, long update_interval);
Light get_light_status();
//void display_garage_object (Garage_Status garage_print);
void change_update_interval(Update_Speed new_speed);


void set_relay(int relay, int on_off) {
  terminal.print("Setting relay ");
  terminal.print(relay);
  terminal.print(" - ");
  terminal.print(switch_pins[relay]);
  terminal.print(" to ");
  if (on_off) {
    if (relay == 0) {
      // TODO put some kind of warning here e.g. flash the light 5 times before closing the door
    }
    terminal.println(" ON");
    digitalWrite(switch_pins[relay], LOW);
  }
  else {
    terminal.println(" OFF");
    digitalWrite(switch_pins[relay], HIGH);
  }
  //digitalWrite(switch_pins[relay],(on_off>0));
  terminal.flush();
}


// This function flashes the onboard LED light.  The period is long for when the link is up and short when it is not
void flashLED() {
  digitalWrite(LED, LOW);  // Turn the LED on (Note that LOW is the voltage level
  // but actually the LED is on; this is because
  // it is active low on the ESP-01)
  delay(50);
  digitalWrite(LED, HIGH);
  //Serial.println("flashLED");
}

/************************************************
  Ultrasound and garage door
*/

// Ultrasonic variables
long duration;

#define DOOR_CLOSED 2300 //threshold position (mm) from ultrasound when door is closed
#define DOOR_CLOSED_MAX 2450 //maximum position (mm) from ultrasound when door is closed
#define DOOR_OPEN 150 //threshold position (mm) from ultrasound when door is open
#define DOOR_OPEN_MIN 120 //threshold position (mm) from ultrasound when door is open
#define TYP_VEL 200 //expected garage door speed (mm/sec) from ultrasound when door is moving
#define MIN_VEL 50 //minimum garage door speed (mm/sec) threshold. Under this threshold the door is considered tobe *not* moving

#define BAD_MEAS_LOW DOOR_OPEN_MIN // any measurement below this threshold is always erroneous
#define BAD_MEAS_HIGH DOOR_CLOSED_MAX // any measurement above this threshold is always erroneous
#define TOTAL_MEAS 10

void updateUltrasonicPosition() {
  // measure the door position and record it into Garage.  Calculate velocity and raise alarms

  // parameters
  long all_measurements[TOTAL_MEAS];  //TODO convert this to int to save memory
  int bad_measurements = 0; //totals bad measurements
  long max_measurement = -100; //longest measurement
  long min_measurement = 100; //shortest measurement
  long current_door_position = 0; //average measurement
  long end_time; //measure end time
  long test_time;  // total test time
  long start_time = micros(); //measure start time
  long current_door_micros; //the approximate middle time of the testing in micros
  int i;

  // first get TOTAL_MEAS distance measurements
  for (i = 0; i <= TOTAL_MEAS - 1; i++)
  {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(echoPin, HIGH, 15000L); // limit to 15000 uSec timeout
    // Calculate and store the distance.  use long to keep accuracy
    all_measurements[i] = microsecondsToMillimeters(duration);
    Serial.println(all_measurements[i]);
    //Serial.println(distance);
  }
  //delay(0);
  Serial.print("Loop finished: ");
  Serial.print(i);
  Serial.print("Loop time (uS)");
  Serial.println(duration);

  end_time = micros();
  test_time = abs(end_time - start_time);  // use abs() to overcome rollover problems
  int total_measurements = 0; //

  for (i = 0; i <= TOTAL_MEAS - 1; i++)
  {
    if ((BAD_MEAS_HIGH < all_measurements[i]) || (all_measurements[i]  < BAD_MEAS_LOW))
    {
      bad_measurements++;
      continue; // exclude this measurement from all calculations
    }

    total_measurements ++;

    // max and min
    if (all_measurements[i] > max_measurement)
    {
      max_measurement = all_measurements[i];
    }
    if (all_measurements[i] < min_measurement)
    {
      min_measurement = all_measurements[i];
    }
    current_door_position += all_measurements[i]; //sum measurements
  }
  current_door_position /= total_measurements;
  current_door_micros = abs(start_time + (test_time / 2)); //TODO check that abs() handles overflow

  Serial.print("Bad measurements: ");
  Serial.println(bad_measurements);

  // Garage object update
  // calculate speed in mm/sec
  //  Serial.println ("Speed calculation");
  //  Serial.println (current_door_position);
  //  Serial.println (garage.door_position);
  //  Serial.println (current_door_micros);
  //  Serial.println (garage.door_position_micros);
  garage.door_speed = float((current_door_position - garage.door_position) * 1e6) / float(abs(current_door_micros - garage.door_position_micros));
  // position and time of position
  garage.door_position_micros =  current_door_micros;
  garage.door_position =  current_door_position;
  // door status
  if (garage.door_speed > 0) {
    garage.door_status = opening;
  }
  else if (garage.door_speed < 0) {
    garage.door_status = closing;
  }
  else if (garage.door_position >= DOOR_CLOSED) {
    garage.door_status = closed;
  }
  else if (garage.door_position <= DOOR_OPEN) {
    garage.door_status = opened;
  } else {
    garage.door_status = alarm;
  }
  garage.door_status_string = status_string[garage.door_status];
}


void updateUltrasonicPositionTest() {
  // parameters
  //long all_measurements[TOTAL_MEAS];  //TODO convert this to int to save memory
  long duration, start_time;
  long current_door_position, current_door_micros;
  int i, total_measurements;

  // Make several measurements,reject obviously wrong ones and average the rest
  start_time = micros();
  current_door_position = 0;
  total_measurements = 0;
  for (i = 0; i <= (TOTAL_MEAS - 1); i++) {
    //pinMode(trigPin, OUTPUT);
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(5);
    digitalWrite(trigPin, LOW);

    // The same pin is used to read the signal from the PING))): a HIGH pulse
    // whose duration is the time (in microseconds) from the sending of the ping
    // to the reception of its echo off of an object.

    //pinMode(echoPin, INPUT);
    duration = pulseIn(echoPin, HIGH);

    // accumulate the position and measurements for averaging afterwards
    current_door_position += microsecondsToMillimeters(duration);
    total_measurements ++;
  }
  // position, speed and time of position.  Sequence of next steps is critical
  current_door_position =  current_door_position / total_measurements;
  current_door_micros = start_time + abs(micros() - start_time) / 2; //TODO check that abs() handles overflow
  garage.door_speed = float((current_door_position - garage.door_position) * 1e6) / float(abs(current_door_micros - garage.door_position_micros));
  // update date in preparation for the next cycle
  garage.door_position = current_door_position;
  garage.door_position_micros = current_door_micros;

  // door status
  if (garage.door_position >= DOOR_CLOSED) {
    garage.door_status = closed;
  }
  else if (garage.door_position <= DOOR_OPEN) {
    garage.door_status = opened;
  }
  else if (garage.door_speed > MIN_VEL) {
    garage.door_status = closing;
  }
  else if (abs(garage.door_speed) > MIN_VEL) {
    garage.door_status = opening;
  }
  else {
    garage.door_status = alarm;
  }
  garage.door_status_string = status_string[garage.door_status];

}

long microsecondsToInches(long microseconds) {
  // According to Parallax's datasheet for the PING))), there are 73.746
  // microseconds per inch (i.e. sound travels at 1130 feet per second).
  // This gives the distance travelled by the ping, outbound and return,
  // so we divide by 2 to get the distance of the obstacle.
  // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
  return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds) {
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the object we
  // take half of the distance travelled.
  return microseconds / 29 / 2;
}

long microsecondsToMillimeters(long microseconds) {
  // The speed of sound is 340 m/s or 2.9 microseconds per millimeter.
  // The ping travels out and back, so to find the distance of the object we
  // take half of the distance travelled.
  return microseconds * 10 / 29 / 2; //multiply by 10 and divide by 29 to avoid rounding errors
}

void printRSSI() {
  // print the SSID of the network you're attached to:

  if (Blynk.connected())
  {
    //    terminal.print("SSID: ");
    //    terminal.println(WiFi.SSID());

    // print your WiFi IP address:
    //    IPAddress ip = WiFi.localIP();
    //    terminal.print("IP Address: ");
    //    terminal.println(ip);

    // print the received signal strength:
    long rssi = WiFi.RSSI();
    terminal.print("signal strength (RSSI): ");
    terminal.print(rssi);
    terminal.println(" dBm");
    //    Serial.print(rssi);
    //    Serial.println(" dBm");
    //    Serial.println(Blynk.connected());
  }
  else
  {
    //terminal.println("Can't check RSSI, system not connected");
    Serial.println("Can't check RSSI, system not connected");
  }
  terminal.flush();
}

void scan_status()
{
  // update status in Garage_Status garage.  (tbd) if a change in state, change update interval

  //Serial.println("scan_status");

  //  updateUltrasonicPosition();
  updateUltrasonicPositionTest();
  // change update interval based on door_status
  //  if (garage.update_speed == slow && garage.door_status != opened && garage.door_status != closed) {
  if (garage.door_status != opened && garage.door_status != closed) {
    change_update_interval(fast);
    //garage.update_speed = fast;
  }
  //  if (garage.update_speed == fast && (garage.door_status == opened || garage.door_status == closed)) {
  if (garage.door_status == opened || garage.door_status == closed) {
    update_blynk(); //one last update before going slow
    change_update_interval(slow);
    //garage.update_speed = slow;
  }
  garage.light = get_light_status();
  garage.temperature = get_temperature();

  //  display_garage_object ();
  display_garage_data ();
}

Light get_light_status() {
  // return light status
  // TODO get this actually working!
#define LIGHT_ON_THRESHOLD 82
  Light l;

  int photocellReading = analogRead(photocellPin);
  l.level = float(map(1024 - photocellReading, 0, 1024, 0, 100));
  l.is_on = l.level > LIGHT_ON_THRESHOLD;
  //map(value, fromLow, fromHigh, toLow, toHigh)
  return l;
}

void change_update_interval(Update_Speed new_speed)
{
  // change update interval to update_interval if current interval != interval
  long update_interval = UPDATE_INTV_FAST; //default interval is FAST
  if (garage.update_speed != new_speed) {
    if (new_speed == slow)  {
      update_interval = UPDATE_INTV_SLOW;
    }
    timer.changeInterval(update_blynk_timer, update_interval);  // set interval to update_interval
    timer.restartTimer(update_blynk_timer);
    garage.update_speed = new_speed;
  }
}

/************************************************

  Temperature

************************************************/

// Setup a oneWire instance to communicate with any OneWire devices
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature temp_Sensor(&oneWire);

float get_temperature() {
  // return temperature
  temp_Sensor.requestTemperatures(); // Send the command to get temperature readings
  return temp_Sensor.getTempCByIndex(0);
}



void display_garage_object ()
{
  // dump contents of garage object here
  Serial.println("Garage object dump ++++++++++++++++++++ +");
  Serial.println("Door position\tSpeed\tStatus");
  Serial.print(garage.door_position);
  Serial.print("\t\t");
  Serial.print(garage.door_speed);
  Serial.print("\t");
  Serial.println(garage.door_status_string);
  Serial.println("Light on\tIntensity ( % )");
  Serial.print(garage.light.is_on ? "yes" : "no");
  Serial.print("\t\t");
  Serial.println(garage.light.level);
  Serial.print("Air temperature(C):  ");
  Serial.println(garage.temperature);
  Serial.print("Update rate: ");
  Serial.println(garage.update_speed ? "fast" : "slow");

}

/************************************************

  Testing

************************************************/
void display_garage_data ()
{
  // dump contents of garage object here in csv format
  static bool fast_test = true; //switch between fast and slow test scans
  static long last_time = 0; // stores last measurement time.
  long now;
  String temp = "";
  if (heading_test_line) {
    Serial.println();
    Serial.println();
    Serial.println("Ms, Door pos, Speed, Status, Temp");
    heading_test_line = false;  //turn of the heading switch
  }
  //ardprintf("test % d % l % c % s % f", l, k, s, j, f)
  //ardprintf(" % l, % d, % d, % s, % f", millis(), garage.door_position, garage.door_speed, garage.door_status_string, garage.temperature);
  now = millis();
  temp = temp + now + ", ";
  temp = temp + garage.door_position + ", ";
  temp = temp + garage.door_speed + ", ";
  temp = temp + garage.door_status_string + ", ";
  temp = temp + garage.temperature + ", ";
  temp += garage.update_speed ? SCAN_INTV_FAST : SCAN_INTV_SLOW ;
  temp += ", ";
  temp += fast_test ? TEST_INTV_FAST : TEST_INTV_SLOW; // add test interval
  temp += ", ";
  temp += (now - last_time); // millis since last test
  Serial.println(temp);

  last_time = now;// update last_time to this time

  // now modify test interval if necessary
  if (garage.door_status != opened && garage.door_status != closed && !fast_test) {
    change_test_interval(TEST_INTV_FAST);
    fast_test = true;
  }
  if ((garage.door_status == opened || garage.door_status == closed) && fast_test) {
    change_test_interval(TEST_INTV_SLOW);
    fast_test = false;
  }
}

void change_test_interval(long new_interval)
{
  // change test interval to new_interval
  timer.changeInterval(test_timer, new_interval);  // set interval to new_interval
  //ardprintf("Changing test timer interval to: % l", new_interval);
  timer.restartTimer(test_timer);
}

/************************************************

  Blynk functions and declarations

  V0=
  V1=WidgetTerminal terminal
  V2=onboard LED ON/OFF
  V3=onboard LED dimmed
  V4=application text box
  V5=relay that controls door
  V6=temperature
  V7=light on_off
  V8=light level
  V9=door position
  V10=door speed
  V11=relay that controls light
  V12 = door status
  V13 = RSSI
  V14 = door up if 1 and down if 0
  V15 = light on if 1 and off if 0

************************************************/


BLYNK_WRITE(V1) {
  // if you type "Marco" into Terminal Widget - it will respond: "Polo:"
  if (String("Marco") == param.asStr()) {
    terminal.println("You said: 'Marco'");
    terminal.println("I said: 'Polo'");
  }
  else if (String("status") == param.asStr() || String("s") == param.asStr())
  {
    terminal_status_update();
  }
  else if (String("update") == param.asStr() || String("u") == param.asStr())
  { terminal_status_update();
    update_terminal = true;
  }
  else if (String("noupdate") == param.asStr() || String("n") == param.asStr())
  {
    update_terminal = false;
    terminal.println();
    terminal.println("Periodic update off");
  }
  else {
    // Send it back
    terminal.print("You said:");
    terminal.write(param.getBuffer(), param.getLength());
    terminal.println();
  }

  // Ensure everything is sent
  terminal.flush();
}

BLYNK_WRITE(V2) {
  // Turn the LED on or off
  int pinData = param.asInt();
  if (pinData) {
    terminal.println("LED on");
    digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)

  }
  else {

    terminal.println("LED off");
    digitalWrite(LED_BUILTIN, HIGH);   // Turn the LED off (Note that HIGH is the voltage level
    // but actually the LED is off; this is because
    // it is active low on the ESP-01)
  }

  // Ensure everything is sent
  terminal.flush();
}

BLYNK_WRITE(V3) {
  // Flash or dim the LED
  float pinData = param.asFloat();
  if (pinData) {
    terminal.print(F("Flash/dim the LED "));
    terminal.print(pinData);
    terminal.println(F(" times and leave it dimmed."));
    //    for (int i = 0; i < pinData; i++) {
    //        digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
    //        // but actually the LED is on; this is because
    //        // it is active low on the ESP-01)
    //        delay(100);                      // Wait for 100 mS
    //        digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
    //        delay(200);                      // Wait for 200 mS (to demonstrate the active low LED)
    //    }
    //analogWrite(LED_BUILTIN, 1023 - pinData); //try to leave the LED in a dimmed state
    // scale the data to give greater dynamic range at the low end (The LED shows more variation at lower numbers)
    double scale_pin_data = pow((pinData / 1023), 4);
    analogWrite(LED_BUILTIN, int(1023 - (scale_pin_data * 1023))); //try to leave the LED in a dimmed state
  }
  else
  {
    terminal.println("LED off");
    digitalWrite(LED_BUILTIN, HIGH);   // Turn the LED off (Note that HIGH is the voltage level
    // but actually the LED is off; this is because
    // it is active low on the ESP-01)
  }

  // Ensure everything is sent
  terminal.flush();
}

BLYNK_WRITE(V5) {
  set_relay(1, param.asInt());  //relay on lightPin
}
BLYNK_WRITE(V11) {
  set_relay(0, param.asInt());  //relay on doorPin
}

BLYNK_WRITE(V14) {
  // open (1) or close(0) the door
  int door_open = param.asInt();
  Serial.print("+++++++++++++++++++++++++++++++++  ");
  Serial.println(door_open);
  if (door_open) {
    // open the door if it's closed
    if (garage.door_status == closed)
    {
      // now open the door while flashing the light 5 times
      Serial.println("Flashing 5 times");
      flash_garage_light(5);
      Serial.println("Toggling the door relay to open");
      toggle_relay(doorPin);
      terminal.println("remote door open initiated");
    }
    else
    {
      // can't open the door,  put the status into terminal
      terminal.print("Can't open door because the door is currently ");
      terminal.println(garage.door_status_string);
      Serial.print("Can't open door because the door is currently ");
      Serial.println(garage.door_status_string);
    }
  }
  else {
    if (garage.door_status == opened) {
      // close the door while flashing the light 5 times
      Serial.println("Flashing 5 times");
      flash_garage_light(5);
      Serial.println("Toggling the door relay to close");
      toggle_relay(doorPin);
      terminal.println("remote door close initiated");
    }
    else
    {
      // can't open the door,  put the status into terminal
      terminal.print("Can't close door because the door is currently ");
      terminal.println(garage.door_status_string);
      Serial.print("Can't close door because the door is currently ");
      Serial.println(garage.door_status_string);
    }
  }
  terminal.flush();
}


void flash_garage_light(int flashes) {
  // flash the garage light for flashes
  for (int i = 0; i < flashes * 2; i++) {
    toggle_relay(lightPin);
  }
}

void toggle_relay(int relay) {
  // toggle a relay for 500 ms
  Serial.print("Toggling the relay :");
  Serial.println(relay);
  digitalWrite(relay, LOW);
  delay(500);
  digitalWrite(relay, HIGH );
  delay(250);
}

BLYNK_WRITE(V15) {
  // turn the garage light on (1) or off (0)
  int light_on = param.asInt();
  Serial.print("+++++++++++++++++++++++++++++++++  ");
  Serial.println(light_on);
  if (light_on) {
    // turn on the light if it's off
    if (!garage.light.is_on)
    {
      Serial.println("Toggling the light relay to on");
      toggle_relay(lightPin);
      terminal.println("garage light remotely turned on");
    }
    else
    {
      // turn on the light if it's on already,  put the status into terminal
      terminal.println("Can't turn light on because it's on already ");
      Serial.println("Can't turn light on because it's on already ");
    }
  }
  else {
    // turn off the light if it's on
    if (garage.light.is_on) {
      Serial.println("Toggling the light relay to off");
      toggle_relay(lightPin);
      terminal.println("garage light remotely turned on");
    }
    else
    {
      // can't turn off the light,  put the status into terminal
      terminal.println("Can't turn light off because it's off already ");
      Serial.println("Can't turn light off because it's off already");
    }
  }
  terminal.flush();
}


void update_blynk() {
  /* update data to Blynk.  Period of update is set elsewhere
    V0 =
    V1 = WidgetTerminal terminal
    V2 = onboard LED ON / OFF
    V3 = onboard LED dimmed
    V4 = application text box
    V5 = relay that controls door
    V6 = temperature
    V7 = light on_off
    V8 = light level
    V9 = door position
    V10 = door speed
    V11 = relay that controls light
    V12 = door status
    V13 = RSSI
  */
  Blynk.virtualWrite(V4, garage.door_status_string);
  Blynk.virtualWrite(V6, garage.temperature);
  Blynk.virtualWrite(V7, garage.light.is_on ? "on" : "off");
  Blynk.virtualWrite(V8, garage.light.level);
  Blynk.virtualWrite(V9, garage.door_position);
  Blynk.virtualWrite(V10, garage.door_speed);
  Blynk.virtualWrite(V12, garage.door_status_string);
  Blynk.virtualWrite(V13, WiFi.RSSI());

  if (update_terminal) {
    terminal_status_update();
  }

}

void terminal_status_update() {
  //  terminal.println("Incoming update != == == == == == == == == == == == == == == == == == == ");
  terminal.println("Garage update ++++++++++++++++++++++++++");
  terminal.print("Door position: ");
  terminal.print(garage.door_position);
  terminal.print(" Speed: ");
  terminal.print(garage.door_speed);
  terminal.print(" Status: ");
  terminal.println(garage.door_status_string);
  terminal.print(garage.light.is_on ? "Light: on  Level: " : "Light: off  Level: ");
  terminal.print(garage.light.level);
  terminal.print(" Air temp(C): ");
  terminal.println(garage.temperature);

  terminal.print("Wifi(dbm): ");
  terminal.print(WiFi.RSSI());
  terminal.println(garage.update_speed ? " Update rate: fast" : " Update rate: slow");
}


/********************************************

  SETUP

*********************************************/
void setup()
{
  // Debug console
  Serial.begin(115200);

  //    Serial.print("Setting Wifi to ");
  //    Serial.print(ssid);
  //    Serial.print(" with password ");
  //    Serial.println(pass);
  //    Serial.print(" Auth key: ");
  //    Serial.println(auth);

  Blynk.begin(auth, ssid, pass);
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk - cloud.com", 80);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);

  //pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  for (int i = 0; i < 2; i++)
  {
    pinMode(switch_pins[i], OUTPUT);
    digitalWrite(switch_pins[i], HIGH);
  }

  pinMode(LED, OUTPUT); // Sets trigPin >> Output
  pinMode(trigPin, OUTPUT); // Sets trigPin >> Output
  pinMode(echoPin, INPUT); // Sets echoPin >> Input
  //  pinMode(buzzerPin, OUTPUT); // Sets buzzerPin >> Output

  heading_test_line = true;

  // This will print Blynk Software version to the Terminal Widget when
  // your hardware gets connected to Blynk Server
  terminal.println(F("Blynk v" BLYNK_VERSION ": Device started"));
  terminal.println(F("------------ -"));
  terminal.println(F("Type 'Marco' and get a reply, or type"));
  terminal.println(F("anything else and get it printed back."));
  terminal.flush();

  // set up timers
  LED_timer = timer.setInterval(LED_INTV_NO_CONNECT, flashLED);  //a timer to flash the LED based on WiFi connectivity.  LED interval to short - default there is no link
  //  RSSI_timer = timer.setInterval(RSSI_INTV, printRSSI);  // print RSSI periodically
  scan_status_timer = timer.setInterval(SCAN_INTV_FAST, scan_status);  // start scan system status in fast mode
  update_blynk_timer = timer.setInterval(UPDATE_INTV_FAST, update_blynk);  // start update Blynk in fast mode
  //test_timer = timer.setInterval(TEST_INTV_SLOW, display_garage_data);  // start test loop in slow mode


  //  Serial.print("Timers initialized: ");
  //  Serial.println(timer.getNumTimers());
  //  Serial.print(" with password ");
  //  Serial.println(pass);
  //  Serial.print(" Auth key: ");
}


/********************************************

  LOOP

*********************************************/
void loop()
{
  Blynk.run();
  timer.run(); // Initiates BlynkTimer
  //  readUltrasonic(); // Uploads ultrasonic data
  //  readUltrasonic_test(); // Uploads ultrasonic data
  //  Serial.println("Blynk running");

  if (Blynk.connected()) {
    // if Blynk connected and previously not connected to server then we have a transition from unconnected
    if (not(connected_to_server)) {
      timer.changeInterval(LED_timer, LED_INTV_CONNECT);  // set LED interval to long - there is a connection
      timer.restartTimer(LED_timer);
      connected_to_server = true;
    }
  }
  else {
    // if Blynk NOT connected and previously connected to server then we have a transition from connected
    if (connected_to_server) {
      timer.changeInterval(LED_timer, LED_INTV_NO_CONNECT);  // set LED interval to short - not connected
      timer.restartTimer(LED_timer);
      connected_to_server = false;
      //Serial.println("Blynk not connected");
    }
  }
}



//#define BLYNK_PRINT Serial