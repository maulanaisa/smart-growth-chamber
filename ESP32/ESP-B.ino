//Dependencies
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

// Replace the next variables with your SSID/Password combination
const char* ssid = "SGC1000L";
const char* password = "chamber1000";

// Add your MQTT Broker IP address
const char* mqtt_server = "34.101.153.146";

WiFiClient espClient;
PubSubClient client(espClient);

 //Variables
bool mode_status ; //mode for actuator, AUTO = 1, MANUAL = 0

//variables
bool heater_status = 0;
bool compressor_status = 0;
bool humidifier_status = 0;
bool minipump_status = 0;
int fan_evap_pwm_value = 73;
unsigned int led_pwm_value = 0;

float temp_sensor = 0;
float humidity_sensor = 0;
float led_sensor = 0;
float temp_sensor_ambient = 0;
int waterlevel_sensor = 0;

float temp_setpoint = 0;
float humidity_setpoint = 0;
float led_setpoint = 0;

//------------------------------------------------------------------------------------------------------------

//Sensor
#include <BH1750.h>
#include <Adafruit_SHT31.h>
const int waterlvl = 34;

Adafruit_SHT31 sht31 = Adafruit_SHT31(); //Create an Adafruit SHT31 object
BH1750 lightMeter(0x23);

//Main Function Sensor Reading and MQTT Publish
void readsensor() {

  float temporary_led;
  if (lightMeter.measurementReady()) {
    temporary_led = lightMeter.readLightLevel();
    if((int(temporary_led) == -1) || (int(temporary_led) == -2)){
    }else{
      led_sensor = temporary_led*1.9952 + 8.3766;
    }
  }

  String test = "nan"; //to avoid "nan" value being displayed
  float temporary_temp = sht31.readTemperature();
  float temporary_humid = sht31.readHumidity();
  
  if(String(temporary_temp) == test){
  }else{
     temp_sensor = temporary_temp;
  }

  if(String(temporary_humid) == test){
  }else{
     humidity_sensor = temporary_humid;
  }
  
  int value   = analogRead(waterlvl);// check water level in humidifier tank
  waterlevel_sensor = value;

   // Publish MQTT data
   char ledsensString[8];
   dtostrf(led_sensor, 1,0, ledsensString);
   client.publish("esp32/ledsens", ledsensString, true);

   char tempsensString[8];
   dtostrf(temp_sensor, 1,1, tempsensString);
   client.publish("esp32/tempsens", tempsensString, true);

   char humsensString[8];
   dtostrf(humidity_sensor, 1,0, humsensString);
   client.publish("esp32/humsens", humsensString, true);

   char watersensString[8];
   itoa(waterlevel_sensor,watersensString,10);
   client.publish("esp32/watersens", watersensString, true);
}

//Time to read sensor output and publish it
unsigned long previousTimeReadSensor = millis();
long timeIntervalReadSensor = 1000;

//------------------------------------------------------------------------------------------------------------

//Display variables in serial monitor
void displayserial() {
  Serial.print("Mode (Auto=1, Manual=0) : ");
  Serial.println(mode_status);
  Serial.println();
  
  Serial.print("heater_status : ");
  Serial.println(heater_status);
  Serial.print("compressor_status : ");
  Serial.println(compressor_status);
  Serial.print("humidifier_status : ");
  Serial.println(humidifier_status);
  Serial.print("minipump_status : ");
  Serial.println(minipump_status);
  Serial.println();
  
  Serial.print("temp_sensor : ");
  Serial.println(temp_sensor);
  Serial.print("humidity_sensor : ");
  Serial.println(humidity_sensor);
  Serial.print("led_sensor  : ");
  Serial.println(led_sensor);
  Serial.print("waterlevel_sensor : ");
  Serial.println(waterlevel_sensor);
  Serial.print("temp_sensor_ambient : ");
  Serial.println(temp_sensor_ambient);
  Serial.println();

  Serial.print("temp_setpoint : ");
  Serial.println(temp_setpoint);
  Serial.print("humidity_setpoint : ");
  Serial.println(humidity_setpoint);
  Serial.print("led_setpoint  : ");
  Serial.println(led_setpoint);
  Serial.println();

  Serial.print("fan_evap_pwm_value  : ");
  Serial.println(fan_evap_pwm_value);
  Serial.print("led_pwm_value : ");
  Serial.println(led_pwm_value);
  Serial.println();


  Serial.println("---------------------------------");
  Serial.println(); 
}

//Time to display variables in serial monitor
unsigned long previousTimeDisplaySerial = millis();
long timeIntervalDisplaySerial = 2000;

//------------------------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(250000);
  setup_wifi(); 
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback); 
  
  //Sensor initialization
  Wire.begin();
  sht31.begin(0x44); //Aktivasi SHTx31 dengan address 0x44
  lightMeter.begin(BH1750::CONTINUOUS_LOW_RES_MODE);
}
//------------------------------------------------------------------------------------------------------------

void loop() {
  if (WiFi.status() == WL_CONNECTED){ //check if mqtt client conected to server
    if(!client.connected()){
      reconnect();
    }
   }else if (WiFi.status() != WL_CONNECTED){ //check if connection is lost
    setup_wifi();
    reconnect();
  }
  client.loop();

  //Main process using millis
  unsigned long currentTime = millis();
    if(currentTime - previousTimeReadSensor > timeIntervalReadSensor){
      previousTimeReadSensor = currentTime;
      readsensor();
    }

    if(currentTime - previousTimeDisplaySerial > timeIntervalDisplaySerial){
       previousTimeDisplaySerial = currentTime;
       displayserial();
    }
}


void setup_wifi() {
   delay(10);
// We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
  
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
  
    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    }
    
void callback(char* topic, byte* message, unsigned int length) { //callback function after mqtt message arrived
    Serial.print("Message arrived on topic: ");
    Serial.print(topic);
    Serial.print(". Message: ");
    String messageTemp;
    
    for (int i = 0; i < length; i++) {
     Serial.print((char)message[i]);
     messageTemp += (char)message[i];
    }
    Serial.println();
   
    if (String(topic) == "esp32/heater") {
    Serial.print("Changing heater to ");
    if(messageTemp == "1"){
      Serial.println("on");
        heater_status  = 1;
    }
    else if(messageTemp == "0"){
      Serial.println("off");
        heater_status = 0;
    }
  }
  
  if(String(topic) == "esp32/cooler"){
    Serial.print("Changing cooler to ");
    if(messageTemp == "1"){
      Serial.println("on");
      compressor_status = 1;
    }
    else if(messageTemp == "0"){
      Serial.println("off");
      compressor_status = 0;
    }
  } 
  
  if(String(topic) == "esp32/humidifier"){
    Serial.print("Changing humidifier to ");
    if(messageTemp == "1"){
      Serial.println("on");
      humidifier_status = 1;
    }
    else if(messageTemp == "0"){
      Serial.println("off");
      humidifier_status = 0;
    }
  }
  
  if(String(topic) == "esp32/led"){
    Serial.print("Changing led to ");
    led_pwm_value = messageTemp.toInt();
    Serial.println(led_pwm_value);
  }
  
  if(String(topic) == "esp32/fan"){
    Serial.print("Changing fan to ");
    fan_evap_pwm_value = messageTemp.toInt();
    Serial.println(fan_evap_pwm_value); 
  }

  if(String(topic) == "esp32/pump"){
    Serial.print("Changing pump to ");
    if(messageTemp == "1"){
      Serial.println("on");
      minipump_status = 1;
    }
    else if(messageTemp == "0"){
      Serial.println("off");
      minipump_status = 0;
    }
  }
  
  if(String(topic) == "esp32/mode"){
    Serial.print("Changing mode to ");
    if(messageTemp == "1"){
      Serial.println("automatic");
       mode_status = 1;
    }
    else if(messageTemp == "0"){
      Serial.println("manual");
       mode_status= 0;
    }
  }

   if(String(topic) == "esp32/setpoint_temperature"){
    Serial.print("Changing setpoint temperature to ");
    temp_setpoint  = messageTemp.toFloat();
    Serial.println(temp_setpoint);
  }
  
   if(String(topic) == "esp32/setpoint_humidity"){
    Serial.print("Changing setpoint humidity to ");
     humidity_setpoint =  messageTemp.toFloat();
     Serial.println(humidity_setpoint);
  }
  
  if(String(topic) == "esp32/setpoint_led"){
    Serial.print("Changing setpoint led to ");
    led_setpoint = messageTemp.toFloat();
    Serial.println(led_setpoint);
  }
  
//    if(String(topic) == "esp32/tempsens"){
////    Serial.print("Temperature : ");
//    temp_sensor =messageTemp.toFloat();
//  }
//  
//   if(String(topic) == "esp32/humsens"){
////    Serial.print("Humidity : ");
//    humidity_sensor =messageTemp.toFloat();
//  }
//
//      if(String(topic) == "esp32/ledsens"){
////    Serial.print("Light : ");
//    led_sensor =messageTemp.toFloat();
//  }
//
//      if(String(topic) == "esp32/watersens"){
////    Serial.print("waterlevel : ");
//    waterlevel_sensor =messageTemp.toInt();
//  }
//  
//  if(String(topic) == "esp32/tempambientsens"){
////    Serial.print("Ambient Temperature : ");
//    temp_sensor_ambient =messageTemp.toFloat();
//  }
  
}

void reconnect() { //reconnect to mqtt server
  // Loop until we're reconnected
  while (!client.connected( )) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    WiFi.mode(WIFI_STA);
    if (client.connect("ESPsgcB")) {

      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/mode",1);
            
      client.subscribe("esp32/heater",1);
      client.subscribe("esp32/cooler",1);
      client.subscribe("esp32/humidifier",1); 
      client.subscribe("esp32/led",1);
      client.subscribe("esp32/fan",1);
      client.subscribe("esp32/pump",1);
      
      client.subscribe("esp32/setpoint_humidity",1);
      client.subscribe("esp32/setpoint_temperature",1);
      client.subscribe("esp32/setpoint_led",1);
      
//      client.subscribe("esp32/ledsens",1);
//      client.subscribe("esp32/tempsens",1);
//      client.subscribe("esp32/humsens",1);
//      client.subscribe("esp32/watersens",1);
//      client.subscribe("esp32/tempambientsens",1);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
