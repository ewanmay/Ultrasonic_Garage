

//#define BLYNK_PRINT Serial


#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "projecttoken";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "ssid";
char pass[] = "password";

// Ultrasonic Sensor Pins
const int trigPin = 0;
const int echoPin = 4;

// Ultrasonic variables
long duration;
int distance;
int average;
int last10[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int cycleCount = 0;
int last10_10Averages[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


// Outputs to Relay Pins
const int garagePin = 5;
const int lightPin = 13;

// Buzzer Pin
const int buzzerPin = 14;
// Attach virtual serial terminal to Virtual Pin V1
WidgetTerminal terminal(V1);

// set up a timer to indicate link status
BlynkTimer timer;
int connected_to_server = 0; //boolean to indicate connection
int timer_number; //reference to connected led timer

// This function flashes the blue light.  The period is long for when the link is up and short when it is not
void myTimerEvent()
#define LED 2
//#define LED LED_BUILTIN
{
  analogWrite(LED, 0);  // Turn the LED on (Note that LOW is the voltage level
  // but actually the LED is on; this is because
  // it is active low on the ESP-01)
  delay(50);
  //digitalWrite(LED, HIGH);
  analogWrite(LED, 1024);
}

// Relay stuff
byte switch_pins[] = {garagePin, lightPin}; // number of gpio to be used as switch/relay control

BLYNK_WRITE(V11) {set_relay(0,param.asInt());} //relay
BLYNK_WRITE(V5) {set_relay(1,param.asInt());} //relay

void set_relay(int relay,int on_off){
  terminal.print("Setting relay ") ;
  terminal.print(relay) ;
  terminal.print("-") ;
  terminal.print(switch_pins[relay]) ;
  terminal.print(" to ") ;
  if (on_off){
    if(relay == 0) {
//      soundBuzzer();
    }
    terminal.println(" ON") ;
    digitalWrite(switch_pins[relay],LOW);
  }
  else {
    terminal.println(" OFF") ;
    digitalWrite(switch_pins[relay],HIGH);
  }
  //digitalWrite(switch_pins[relay],(on_off>0));
  terminal.flush();
}

// You can send commands from Terminal to your hardware. Just use
// the same Virtual Pin as your Terminal Widget
BLYNK_WRITE(V1)
{
  // if you type "Marco" into Terminal Widget - it will respond: "Polo:"
  if (String("Marco") == param.asStr()) {
    terminal.println("You said: 'Marco'") ;
    terminal.println("I said: 'Polo'") ;
  } else {
    // Send it back
    terminal.print("You said:");
    terminal.write(param.getBuffer(), param.getLength());
    terminal.println();
  }

  // Ensure everything is sent
  terminal.flush();
}

BLYNK_WRITE(V2)
{
  // Turn the LED on
  int pinData = param.asInt();
  if (pinData) {
    terminal.println("LED on") ;
    digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)

  } else {

    terminal.println("LED off") ;
    digitalWrite(LED_BUILTIN, HIGH);   // Turn the LED off (Note that HIGH is the voltage level
    // but actually the LED is off; this is because
    // it is active low on the ESP-01)
  }

  // Ensure everything is sent
  terminal.flush();
}

BLYNK_WRITE(V3)
{
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
    double scale_pin_data = pow((pinData/1023),4);
    analogWrite(LED_BUILTIN, int(1023 - (scale_pin_data* 1023))); //try to leave the LED in a dimmed state
  } else {

    terminal.println("LED off") ;
    digitalWrite(LED_BUILTIN, HIGH);   // Turn the LED off (Note that HIGH is the voltage level
    // but actually the LED is off; this is because
    // it is active low on the ESP-01)
  }

  // Ensure everything is sent
  terminal.flush();
}

void readUltrasonic() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2;
  average = updateLast10(distance);
  int moving = determineMoving();
  // Prints the distance on the Serial Monitor
  if(moving == 0) {
    Blynk.virtualWrite(V4, "Opening"); 
  }
  else if (moving == 1) {
    Blynk.virtualWrite(V4, "Closing");
  }
  else if (average > 200) { // For distances greater than 2 meters, the door is assumed closed
    Serial.println("Writing: Closed");
    Blynk.virtualWrite(V4, "Closed");
  } else if (average > 20 && average < 200 ) {
    Blynk.virtualWrite(V4, "Stuck");
  } else {
    Serial.println("Writing: Open");
    Blynk.virtualWrite(V4, "Open");
  }
}

int updateLast10(int distance) {
  int i;
  int inputs = 1;
  Serial.println("Last 10");
  int average = 0;
  for (i = 0; i < 9; i++) {
    if (last10[i] != 0 && last10[i] < 400) {
      average = average + last10[i];
      inputs = inputs + 1;
    }
    last10[i] = last10[i + 1];
    Serial.println(i);
    Serial.println(last10[i]);
  }
  last10[9] = distance;
  if(distance < 400) {
    inputs = inputs + 1;
    average = average + distance;
  }

  
  Serial.println("Average: ");
  average = average / inputs;
  
  //for added accuracy, let's take the averages of the last 10 completely different averages
  if(cycleCount == 10) {
    updateLast10Averages(average);
    cycleCount = 1;
  } else {
    cycleCount = cycleCount + 1;
  }
  Serial.println(average);
  return average;
}

int determineMoving() {
  if(last10_10Averages[0] == 0 && last10_10Averages[9] == 0 || (abs(last10_10Averages[0] - last10_10Averages[9]) >= 1000) ) {
    return 2;
  }
  else {
    Serial.println("Moving parameters");
    Serial.println(last10_10Averages[0]);
    Serial.println(last10_10Averages[9]);
    float rateOfChange = float(last10_10Averages[0])/float(last10_10Averages[9]);
    Serial.println(rateOfChange);
    Serial.println(last10_10Averages[0]);
    Serial.println(last10_10Averages[9]); 
    if(rateOfChange == 0.00) {
      return 2;
    }
    else if(rateOfChange > 1.4) {
      return 0;
    }
    else if(rateOfChange < 0.6) {
      return 1;
    }
    else {
      return 2;
    }
  }
};

void updateLast10Averages(int newAverage) { 
  int i;
  int inputs = 1;
  Serial.println("Last 10 full averages");
  int average = 0;
  for (i = 0; i < 9; i++) {
    if (last10_10Averages[i] != 0 && last10_10Averages[i] < 400) {
      average = average + last10_10Averages[i];
      inputs = inputs + 1;
    }
    last10_10Averages[i] = last10_10Averages[i + 1];
    Serial.println(i);
    Serial.println(last10_10Averages[i]);
  }
  last10_10Averages[9] = newAverage;
  if(distance < 400) {
    inputs = inputs + 1;
    average = average + distance;
  }
}                                                                

void printRSSI() {// set LED interval to short - default there is no link
  // print the SSID of the network you're attached to:
  terminal.print("SSID: ");
  terminal.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  terminal.print("IP Address: ");
  terminal.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  terminal.print("signal strength (RSSI):");
  terminal.print(rssi);
  terminal.println(" dBm");
  terminal.flush();
}

void setup()
{
  // Debug console
  Serial.begin(115200);

  Blynk.begin(auth, ssid, pass);
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 80);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);

  //pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  for (int i = 0;i<2;i++){
    pinMode(switch_pins[i], OUTPUT);
    digitalWrite(switch_pins[i],HIGH);
  }

  // This will print Blynk Software version to the Terminal Widget when
  // your hardware gets connected to Blynk Server
  terminal.println(F("Blynk v" BLYNK_VERSION ": Device started"));
  terminal.println(F("-------------"));
  terminal.println(F("Type 'Marco' and get a reply, or type"));
  terminal.println(F("anything else and get it printed back."));
  terminal.flush();
  
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  pinMode(buzzerPin, OUTPUT); // Sets the buzzerPin to Output
  
  timer_number = timer.setInterval(500L, myTimerEvent);  // set LED interval to short - default there is no link
  timer.setInterval(5000L, printRSSI);
}

//void soundBuzzer() {
//  tone(buzzerPin, 1000);
//  delay(1000);
//  tone(buzzerPin, 1000);
//  delay(1000);
//  tone(buzzerPin, 1000);
//  delay(1000);
//}

void loop()
{
  Blynk.run();
  timer.run(); // Initiates BlynkTimer 
  if (Blynk.connected()) {
    // if Blynk connected and previously not connected to server then we have a transition
    if (not(connected_to_server)) {
      timer.changeInterval(timer_number, 5000L);  // set LED interval to long - there is a connection
      timer.restartTimer(timer_number);
      connected_to_server = true;
    }
    readUltrasonic(); // Uploads ultrasonic data 
  }
  else {
    // if Blynk NOT connected and previously connected to server then we have a transition
    if (connected_to_server) {
      timer.changeInterval(timer_number, 500L);  // set LED interval to short - not connected
      timer.restartTimer(timer_number);
      connected_to_server = false;
    }
  }
}
