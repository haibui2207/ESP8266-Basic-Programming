#include <ESP8266WiFi.h>
#include<PubSubClient.h>
#include "wifi_info.h"
                            
const char* mqtt_server=IP_MQTT;          
const char* ssid = SSID;
const char* password = WPA_KEY;

int led_bath = 5;
int led_bed1 = 4;
int led_bed2 = 14;
int led_kitchen = 12;
int led_living =  13;

WiFiClient espclient;                                                   
PubSubClient client(espclient);   

//SETUP()
void setup() {
  // Set pinMode 
  pinMode(led_bath,OUTPUT);                                               
  pinMode(led_bed1,OUTPUT);    
  pinMode(led_bed2,OUTPUT);    
  pinMode(led_kitchen,OUTPUT);    
  pinMode(led_living,OUTPUT);    
  
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
      case 'L' : 
        digitalWrite(led_living,HIGH);
        client.publish("ledState","Light's living room has been turned on");
        break;
      case 'l' : 
        digitalWrite(led_living,LOW);
        client.publish("ledState","Light's living room has been turned off");
        break;
      case 'B' : 
        digitalWrite(led_bed1,HIGH);
        client.publish("ledState","Light's bed room 1 has been turned on");
        break;
      case 'b' : 
        digitalWrite(led_bed1,LOW);
        client.publish("ledState","Light's bed room 1 has been turned off");
        break;
      case 'S' : 
        digitalWrite(led_bed2,HIGH);
        client.publish("ledState","Light's bed room 2 has been turned on");
        break;
      case 's' : 
        digitalWrite(led_bed2,LOW);
        client.publish("ledState","Light's bed room 2 has been turned off");
        break;
      case 'K' : 
        digitalWrite(led_kitchen,HIGH);
        client.publish("ledState","Light's kitchen has been turned on");
        break;
      case 'k' : 
        digitalWrite(led_kitchen,LOW);
        client.publish("ledState","Light's kitchen has been turned off");
        break;
      case 'H' : 
        digitalWrite(led_bath,HIGH);
        client.publish("ledState","Light's bathroom has been turned on");
        break;
      case 'h' : 
        digitalWrite(led_bath,LOW);
        client.publish("ledState","Light's bathroom has been turned off");
        break;      
      case 'A' : 
        All_led('A');
        client.publish("ledState","Light's all room has been turned on");
        break;
      case 'a' : 
        All_led('a');
        client.publish("ledState","Light's all room has been turned off");
        break; 
    }
  }
  else{
    Serial.print("Fail to receive payload !!! - payload : ");
    Serial.println(charMESSAGE);    
  } 
}
//ALL ON or OFF
void All_led(char c){
  if(c == 'A'){
    digitalWrite(led_living,HIGH);
    digitalWrite(led_bed1,HIGH);
    digitalWrite(led_bed2,HIGH);
    digitalWrite(led_kitchen,HIGH);
    digitalWrite(led_bath,HIGH);
  }
  else{
    digitalWrite(led_living,LOW);
    digitalWrite(led_bed1,LOW);
    digitalWrite(led_bed2,LOW);
    digitalWrite(led_kitchen,LOW);
    digitalWrite(led_bath,LOW);
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
    Serial.print("Can't connect. Reconnecting...\n");
    setup_wifi();                                                             
  }
  while(!client.connected()){
    if(client.connect("HAIBUIClientLedControl")){
      Serial.println("Connected to MQTT Server"); 
      client.subscribe("ledControl");                       
      Serial.println("Subscribe topic ledControl success");  
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
  if(!client.connected()){       
     listenMQTT();
  }
  else{
    client.loop();
  }    
//  delay(500);
}
