#include <ESP8266WiFi.h>
#include<PubSubClient.h>
#include "wifi_info.h"
#include "DHT.h"  
         
const char* mqtt_server=IP_MQTT;            
const char* ssid = SSID;
const char* password = WPA_KEY;

char* mes;                           //Set message to publish

bool check;                          //điều kiện on off bell
bool highMoveSensor = false;         //on off motion sensor
int counter = 0;                     //định thời cho bell

int pinLed = 14;
int pinFire = 12;
int pinMove = 4;                                                         
int pinSound = 5;
const int DHTPIN = 10; 
const int DHTTYPE = DHT11;

//Declare Libs
WiFiClient espclient;                                                     //Declare WiFiClient
PubSubClient client(espclient);     //Declare PubSubClient

DHT dht(DHTPIN, DHTTYPE);

//SETUP()
void setup() {
  dht.begin();
  // Set pinMode 
  pinMode(DHTPIN,INPUT);
  pinMode(pinFire,INPUT);                                                //Fire sensor
  pinMode(pinMove, INPUT);
  pinMode(pinLed,OUTPUT);                                                 //led
  pinMode(pinSound,OUTPUT);                                               //Sound
  digitalWrite(pinSound,HIGH);
  
  //Set Baurate
  Serial.begin(115200);                                                   //Baurate 115200 
  
  //Setup wifi
  setup_wifi();                                                           //Setup wifi

  //Setup mqtt server and callback funtion of client
  client.setServer(mqtt_server, 1883);                                    //Setup mqtt server of client
  client.setCallback(callback);                                           //Setup callback funtion of client  

  //Subcribe Topic from MQTT Server
  listenMQTT();                                                           //Subcribe Topic from MQTT Server
}

//SETUP WIFI
void setup_wifi(){
  Serial.println();
  Serial.print("Connecting to ");                                         //Print Serial monitoring
  Serial.println(ssid);
   
  //Connect to Wifi
  WiFi.begin(ssid,password);                                              //Connect to Wifi

  //Check if connect fail
  while(WiFi.status()!=WL_CONNECTED){                                     //Check if connect fail
    delay(100);
    Serial.println("Can't connect. Reconnecting...\n");
  }
  //If connect success
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());                                         //Show Local IP   
}

//CALLBACK()
void callback(char* topic,byte* payload,unsigned int lengthPayload){    
  Serial.print("Message arrived from topic : [");
  Serial.print(topic);
  Serial.println("]");

  //Print Length of payload  
  Serial.print("Length of payload : ");
  Serial.println(lengthPayload);  
  
  //Handle payload content
  char* charMESSAGE = handlePayload(payload,lengthPayload);
  if(lengthPayload > 0){
    Serial.print("MESSAGE RECEIVED IS : ");
    Serial.println(charMESSAGE);
    switch(*charMESSAGE){
      case '0':
        //Turn off Speaker and Led
        digitalWrite(pinSound,HIGH);                                        //reverse
        digitalWrite(pinLed,LOW);
        check = false;
        if(client.connect("HAIBUIClientMoveFireControl")){  
           client.publish("speakerState","OFF");
           client.publish("moveState","Normal"); 
           client.publish("fireState","Normal"); 
           client.publish("DHT11State","Normal");
           break;
        }  
        else{
          Serial.println("Can't send message to MQTT Server");
          break;
        }       
      case '1':
        highMoveSensor = true;
        Serial.print("highMoveSensor: ");
        Serial.println(highMoveSensor);
        break;
      case '2':
        highMoveSensor = false;
        Serial.println("Can't send message to MQTT Server");
        break;
    }
  }
  else{
    Serial.print("Fail to receive payload !!! - payload : ");
    Serial.println(charMESSAGE);    
  } 
}

//HANDLEPAYLOAD()
char* handlePayload(byte* payload,unsigned int lengthPayload ){
  char* charMESSAGE = new char[lengthPayload];  
  for(int i=0;i<lengthPayload;i++){                                       //Read values from payload
      Serial.print("Value received : ");
      Serial.println(payload[i]);                                         //Print ASCII 
      charMESSAGE[i] = (char)payload[i];                                                 
    }
  return charMESSAGE;
}

//LISTENMQTT()
void listenMQTT(){
  while(WiFi.status()!=WL_CONNECTED){
    delay(100);
    Serial.print("Can't connect. Reconnecting...\n");
    setup_wifi();                                                             
  }
  while(!client.connected()){
    if(client.connect("HAIBUIClientMoveFireControl")){
      Serial.println("Connected to MQTT Server"); 
      client.subscribe("turnoffSpeaker");            
      client.subscribe("turnonMotionSensor");           
      Serial.println("Subscribe topic turnoffSpeaker success");  
//      client.subscribe("messagetopic");                                    //SUBSCRIBE Topic name "messagetopic" to Receive Payload from topic when someone PUBLISH a payload on Topic 
//      Serial.println("Subscribe topic messagetopic success");       
   }
    else{
      Serial.print("Failed,Can't connect to MQTT Server , STATE = ");
      Serial.println(client.state());
      delay(100);
    }
  } 
}

//LOOP()
void loop() {
  float h = dht.readHumidity();    //Đọc độ ẩm
  float t = dht.readTemperature(); //Đọc nhiệt độ
  float f = dht.readTemperature(true); //Đọc nhiệt độ *F
  static char humi[10];
  static char temp[10];
  dtostrf(h,6,2,humi);
  dtostrf(t,6,2,temp);
  Serial.println();
  if ( t > 25 ){
    mes = "Warning . Temperature is so high";
    check = true;
    Serial.print("Nhiet do lon hon ");
    Serial.print(t);
    Serial.println("*C");   
    if(client.connect("HAIBUIClientMoveFireControl")){  
      client.publish("speakerState","ON");  
      client.publish("DHT11State",mes);        
    } 
    else{
      Serial.println("Move sensor : Can't send message to MQTT Server");
    } 
  }
  Serial.print("Humidity: ");
  Serial.println(h);
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print("*C- ");
  Serial.print(f);
  Serial.println("*F ");
  if(counter%10 == 0){
    if(client.connect("HAIBUIClientMoveFireControl")){  
     client.publish("DHT11Temp",temp);
     client.publish("DHT11Hum",humi);           
    } 
    else{
      Serial.println("Move sensor : Can't send message to MQTT Server");
    } 
  }
  
  
  //ONLY ON, OFF via control
  if ((highMoveSensor == true) && (digitalRead(pinMove) == HIGH))
  {          
    mes = "Warning have moving!!!";  
    check = true;
//    digitalWrite(pinSound,LOW);
//    digitalWrite(pinLed,HIGH);   
    Serial.println("Warning have moving");
    if(client.connect("HAIBUIClientMoveFireControl")){  
      client.publish("speakerState","ON");  
      client.publish("moveState",mes);        
    } 
    else{
      Serial.println("Move sensor : Can't send message to MQTT Server");
    }
  }
  else{
    Serial.println("Move sensor : Normal");    
  }
  //ONLY ON , OFF via control
  if (digitalRead(pinFire) == LOW)
  {        
    mes = "Warning have fire!!!";  
    check = true; 
//    digitalWrite(pinSound,HIGH);
//    digitalWrite(pinLed,LOW);          
    Serial.println("Warning have fire");
    if(client.connect("HAIBUIClientMoveFireControl")){     
       client.publish("speakerState","ON");
       client.publish("fireState",mes);       
    }  
    else{
      Serial.println("Fire sensor : Can't send message to MQTT Server");
    }
  }
  else{
    Serial.println("Fire sensor : Normal");    
  }  

  if(check == true){
    if(counter%10 == 0){
      digitalWrite(pinSound,LOW);
      digitalWrite(pinLed,HIGH);  
    }
    else if(counter %5 == 0){
      digitalWrite(pinSound,HIGH);
      digitalWrite(pinLed,LOW);  
    }
  }
  
  if(!client.connected()){       
     listenMQTT();
  }
  else{
    client.loop();
  } 
  counter = (counter+1)%40;
  Serial.print("Counter: ");
  Serial.println(counter);
  delay(100);           //Hủy delay để test realtime
}
