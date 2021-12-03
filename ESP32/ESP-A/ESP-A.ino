//Dependencies
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

WiFiClient espClient;
PubSubClient client(espClient);

// Replace the next variables with your SSID/Password combination
const char* ssid = "SGC1000L";
const char* password = "chamber1000";

// Add your MQTT Broker IP address
const char* mqtt_server = "34.101.153.146";

//Time to publish data using MQTT
unsigned long previousTimeSenddatamqtt =millis();  
long timeIntervalSenddatamqtt = 500;

//Variables
bool mode_status ; //mode for actuator, AUTO = 1, MANUAL = 0

//variables
bool heater_status = 0;
bool compressor_status = 0;
bool humidifier_status = 0;
bool minipump_status = 0;
int fan_evap_pwm_value = 73;
unsigned int led_pwm_value = 0;

bool temp_or_humidity_turn = 1; //0 for humidity control algorithm turn, 1 for temperature  control algorithm turn

bool heater_status_temp; //temporary variables
bool compressor_status_temp;
bool humidifier_status_temp ;
bool minipump_status_temp;
int fan_evap_pwm_value_temp;
unsigned int led_pwm_value_temp;

float temp_sensor = 0;
float humidity_sensor = 0;
float led_sensor = 0;
float temp_sensor_ambient = 0;
int waterlevel_sensor = 0;

float temp_setpoint = 0;
float humidity_setpoint = 0;
float led_setpoint= 0;

//Hysterisis value to avoid sudden change
float temp_hysterisis = 1.5;
float humidity_hyterisis = 5;
float led_hysterisis = 10;

//------------------------------------------------------------------------------------------------------------

//Temperature and Humidity Control
#include <RBDdimmer.h>
#define PIN_HEATER 18
#define PIN_COMPRESSOR 22
#define PIN_FAN_EVAP 21
#define ZC 19
#define PIN_HUMIDIFIER 14
#define PIN_FAN_HUMIDIFIER 32
dimmerLamp dimmer_Heater(PIN_HEATER, ZC);
dimmerLamp dimmer_Fan_Evap(PIN_FAN_EVAP,ZC);
dimmerLamp dimmer_Compressor(PIN_COMPRESSOR,ZC);

//Time to control temperature and humidity
unsigned long previousTimeTempHumidControl = millis();
long timeIntervalTempHumidControl = 50;

//Time to switch between temperature oriented control or humidity oriented control (auto mode)
unsigned long previousTimeSwitching = millis();
long timeIntervalSwitching = 180000; //3 minutes

void tempandhumid(){  //Function to control temperature and humidity
    unsigned long currentTime2 = millis();
    
    dimmer_Fan_Evap.setPower(fan_evap_pwm_value); //set the power of evaporator fan
    dimmer_Heater.setPower(40); //set the power of the heater
    dimmer_Compressor.setPower(67); //set the power of the compressor
  
    if(mode_status){  //if mode --> automatic
      if(temp_or_humidity_turn){ //check whether temperature or humidity control in charge : 1 for temp control and 0 for humidity control
        if(temp_setpoint < temp_sensor_ambient){  //if temp setpoint below ambient temp
            if(temp_sensor > temp_setpoint + temp_hysterisis){
            dimmer_Heater.setState(OFF);  //set the heater off
            dimmer_Compressor.setState(ON); //set the compressor on
            digitalWrite(PIN_HUMIDIFIER,OFF); //active low
            digitalWrite(PIN_FAN_HUMIDIFIER,OFF); //active low
            heater_status = 0;
            compressor_status = 1;
            humidifier_status = 1;
            fan_evap_pwm_value = 73;
          }else if(temp_sensor < temp_setpoint - temp_hysterisis){
            dimmer_Heater.setState(OFF);
            dimmer_Compressor.setState(OFF);
            digitalWrite(PIN_HUMIDIFIER,ON); //active low
            digitalWrite(PIN_FAN_HUMIDIFIER,ON); //active low
            heater_status = 0;
            compressor_status = 0;
            humidifier_status = 0;
            fan_evap_pwm_value = 73;
          }else{  //when temperature reached
            if((currentTime2 - previousTimeSwitching > timeIntervalSwitching) && (temp_sensor < temp_setpoint)){ //check if it's time to switch into humidity control mode and
               temp_or_humidity_turn = 0; //switch to humidity control mode                                      //if temperature is below setpoint (we already know it's 
               previousTimeSwitching = currentTime2;                                                             //active cooling mode because temp setpoint is below the ambient)
            }
          }
        } else{ //if temp setpoint above ambient temp
            if(temp_sensor > temp_setpoint + temp_hysterisis){
            dimmer_Heater.setState(OFF);
            dimmer_Compressor.setState(OFF);
            digitalWrite(PIN_HUMIDIFIER,ON); //active low
            digitalWrite(PIN_FAN_HUMIDIFIER,ON); //active low
            heater_status = 0;
            compressor_status = 0;
            humidifier_status = 0;
            fan_evap_pwm_value = 73;
          }else if(temp_sensor < temp_setpoint - temp_hysterisis){
            dimmer_Heater.setState(ON);
            dimmer_Compressor.setState(OFF);
            digitalWrite(PIN_HUMIDIFIER,OFF); //active low
            digitalWrite(PIN_FAN_HUMIDIFIER,OFF); //active low
            heater_status = 1;
            compressor_status = 0;
            humidifier_status = 1;
            fan_evap_pwm_value = 73;
          }else{
            if((currentTime2 - previousTimeSwitching > timeIntervalSwitching) && (temp_sensor > temp_setpoint)){ //check if it's time to switch into humidity control mode and
               temp_or_humidity_turn = 0;//switch to humidity control mode                                      //if temperature is below setpoint (we already know it's
               previousTimeSwitching = currentTime2;                                                            //active heating mode because temp setpoint is above the ambient)
            }
          }
        }

     }else { //if it's humidity control mode
        if(humidity_sensor > humidity_setpoint + humidity_hyterisis){
          digitalWrite(PIN_HUMIDIFIER,ON); //active low
          digitalWrite(PIN_FAN_HUMIDIFIER,ON); //active low
          dimmer_Heater.setState(ON);
          dimmer_Compressor.setState(ON);
          humidifier_status = 0;
          fan_evap_pwm_value = 73;
          heater_status = 1;
          compressor_status = 1;
        }else if(humidity_sensor < humidity_setpoint - humidity_hyterisis ){
          digitalWrite(PIN_HUMIDIFIER,OFF); //active low
          digitalWrite(PIN_FAN_HUMIDIFIER,OFF); //active low
          dimmer_Heater.setState(OFF);
          dimmer_Compressor.setState(OFF);
          humidifier_status = 1;
          fan_evap_pwm_value = 73;
          heater_status = 0;
          compressor_status = 0;
        }else{
          if(currentTime2 - previousTimeSwitching > timeIntervalSwitching){ //check if it's time to switch into temp control mode
               temp_or_humidity_turn = 1;//switch to humidity control mode
               previousTimeSwitching = currentTime2;
            }
        }   
      }  
  }else { //if mode --> manual
    
    if(heater_status){
      dimmer_Heater.setState(ON);
    }else{
      dimmer_Heater.setState(OFF);
    }

    if(compressor_status){
      dimmer_Compressor.setState(ON);
    }else {
      dimmer_Compressor.setState(OFF);
    }

    dimmer_Fan_Evap.setPower(fan_evap_pwm_value);

    if(humidifier_status){
      digitalWrite(PIN_HUMIDIFIER,OFF); //active low
      digitalWrite(PIN_FAN_HUMIDIFIER,OFF); //active low
    }else {
      digitalWrite(PIN_HUMIDIFIER,ON);//active low
      digitalWrite(PIN_FAN_HUMIDIFIER,ON);//active low
    }
  }

}

//------------------------------------------------------------------------------------------------------------

//LED Control
#define PIN_LED_1 25
#define PIN_LED_2 26
#define PIN_LED_3 27

#define LED_PWM_RESOLUTION 8
#define PWM_FREQ 5000
#define LED_CHANNEL 0

//Time to control led
unsigned long previousTimeLedControl = millis();
long timeIntervalLedControl = 100;

//LED Control
void ledcontrol(){
  if(mode_status){
    unsigned int ibuffer=led_pwm_value;
    ledcWrite(LED_CHANNEL,map(ibuffer,0,100,0,255)); //project 0-100 range into 0-255 pwm value range
    if(led_sensor < led_setpoint - led_hysterisis){
      ibuffer += 1;
      ibuffer = constrain(ibuffer,0,100); //limit lowest and highest led value
    }else if(led_sensor > led_setpoint + led_hysterisis){
      ibuffer -= 1;
      ibuffer = constrain(ibuffer,0,100);
    }else{
      
    }
    led_pwm_value = ibuffer;
    
  }
   
  else{
    ledcWrite(LED_CHANNEL,map(led_pwm_value,0,100,0,255)); //output pwm value
    } 
  }

//------------------------------------------------------------------------------------------------------------

//Minipump control
  #define PIN_MINIPUMP 12

  //Time to control minipump
  unsigned long previousTimeMinipumpControl = millis();
  long timeIntervalMinipumpControl = 2000;

  //Minipump Control
void minipumpcontrol(){ 
    if (waterlevel_sensor){
      digitalWrite(PIN_MINIPUMP,HIGH); //active low
    }
    else {      
      digitalWrite(PIN_MINIPUMP,LOW); // active low
    }
}

//------------------------------------------------------------------------------------------------------------

//Display variables in serial monitor
void displayserial(){
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

  Serial.print("temp_humidity_turns : ");
  Serial.println(temp_or_humidity_turn);
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
  setup_wifi(); //set up connection to wifi
  client.setServer(mqtt_server, 1883);  //set up mqtt client
  client.setCallback(callback);

  //Temperature and Humidity Control
  pinMode(PIN_HUMIDIFIER, OUTPUT);
  pinMode(PIN_FAN_HUMIDIFIER, OUTPUT);
  dimmer_Heater.begin(NORMAL_MODE, OFF);
  dimmer_Heater.setPower(60);
  dimmer_Fan_Evap.begin(NORMAL_MODE, ON);
  dimmer_Fan_Evap.setPower(fan_evap_pwm_value);
  dimmer_Compressor.begin(NORMAL_MODE, OFF);
  dimmer_Compressor.setPower(67);

  //LED Control
  // configure LED PWM functionalitites
  ledcSetup(LED_CHANNEL, PWM_FREQ, LED_PWM_RESOLUTION);
  
  //attach the channel to the GPIO to be controlled
  ledcAttachPin(PIN_LED_1, LED_CHANNEL);
  ledcAttachPin(PIN_LED_2, LED_CHANNEL);
  ledcAttachPin(PIN_LED_3, LED_CHANNEL);

  //Minipump Control
  pinMode(PIN_MINIPUMP, OUTPUT);
  digitalWrite(PIN_MINIPUMP,HIGH);// turn off minipump  

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
  if(currentTime - previousTimeTempHumidControl > timeIntervalTempHumidControl){
    previousTimeTempHumidControl = currentTime;
    tempandhumid();    
  }

  if(currentTime - previousTimeLedControl > timeIntervalLedControl){
    previousTimeLedControl = currentTime;
    ledcontrol();
  }

  if(currentTime - previousTimeMinipumpControl > timeIntervalMinipumpControl){
    previousTimeMinipumpControl = currentTime;
    minipumpcontrol();
  }

  if(currentTime - previousTimeSenddatamqtt > timeIntervalSenddatamqtt){
    previousTimeSenddatamqtt = currentTime;
    Senddatamqtt();
  }

    if(currentTime - previousTimeDisplaySerial > timeIntervalDisplaySerial){
    previousTimeDisplaySerial = currentTime;
    displayserial();
  }
}

//------------------------------------------------------------------------------------------------------------

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

void callback(char* topic, byte* message, unsigned int length) {  //callback function after mqtt message arrived
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
  }
  
   if(String(topic) == "esp32/setpoint_humidity"){
    Serial.print("Changing setpoint humidity to ");
     humidity_setpoint =  messageTemp.toFloat();
  }
  
  if(String(topic) == "esp32/setpoint_led"){
    Serial.print("Changing setpoint led to ");
    led_setpoint =messageTemp.toFloat();
  }
  
    if(String(topic) == "esp32/tempsens"){
//    Serial.print("Temperature : ");
    temp_sensor =messageTemp.toFloat();
  }
  
   if(String(topic) == "esp32/humsens"){
//    Serial.print("Humidity : ");
    humidity_sensor =messageTemp.toFloat();
  }

      if(String(topic) == "esp32/ledsens"){
//    Serial.print("Light : ");
    led_sensor =messageTemp.toFloat();
  }

      if(String(topic) == "esp32/watersens"){
//    Serial.print("waterlevel : ");
    waterlevel_sensor =messageTemp.toInt();
  }
  
  if(String(topic) == "esp32/tempambientsens"){
//    Serial.print("Ambient Temperature : ");
    temp_sensor_ambient =messageTemp.toFloat();
  }
}

void reconnect() {  //reconnect to mqtt server
  // Loop until we're reconnected
  while (!client.connected( )) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    WiFi.mode(WIFI_STA);
    if (client.connect("ESPsgcA")) {

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
      
      client.subscribe("esp32/ledsens",1);
      client.subscribe("esp32/tempsens",1);
      client.subscribe("esp32/humsens",1);
      client.subscribe("esp32/watersens",1);
      client.subscribe("esp32/tempambientsens",1);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//Publish MQTT data
void Senddatamqtt(){
    //publish only when there are changes to actuator state
    if (heater_status_temp != heater_status ) {
      char heaterString[2];
      itoa(heater_status,heaterString,10);
      client.publish("esp32/heater", heaterString, true);
    }
    
    if (compressor_status_temp != compressor_status ) {
      char compressorString[2];
      itoa(compressor_status,compressorString,10);
      client.publish("esp32/cooler", compressorString, true);
    }
    
    if (humidifier_status_temp != humidifier_status ) {
      char humidifierString[2];
      itoa(humidifier_status,humidifierString,10);
      client.publish("esp32/humidifier", humidifierString, true);
    }
    
    if (fan_evap_pwm_value != fan_evap_pwm_value_temp ) {
      char fan_evap_pwm_valueString[2];
      dtostrf(fan_evap_pwm_value, 1, 0, fan_evap_pwm_valueString);
      client.publish("esp32/fan", fan_evap_pwm_valueString, true); 
    }
    
    if (led_pwm_value != led_pwm_value_temp ) {
      char ledString[8];
      dtostrf(led_pwm_value, 1, 0, ledString);
      client.publish("esp32/led", ledString, true); 
    }

    if (minipump_status_temp != minipump_status) {
      char minipumpString[2];
      itoa(minipump_status,minipumpString,10);
      client.publish("esp32/pump", minipumpString, true);
    }

  heater_status_temp = heater_status;
  compressor_status_temp = compressor_status;
  humidifier_status_temp  = humidifier_status;
  fan_evap_pwm_value_temp = fan_evap_pwm_value;
  led_pwm_value_temp = led_pwm_value; 
  minipump_status_temp = minipump_status;
 }
