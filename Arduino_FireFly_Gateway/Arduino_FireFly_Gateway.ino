 /*

IBM Usefull URLs:
https://developer.ibm.com/recipes
https://quickstart.internetofthings.ibmcloud.com
https://<Organization_ID_HERE>.internetofthings.ibmcloud.com/dashboard

*/

#include <UIPEthernet.h>
#include <PubSubClient.h>
// include the library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Update these with values suitable for your network.
byte mac[] = { 0xF0, 0xAE, 0x0C, 0x9F, 0x84, 0x14 }; //change to unique MAC
//update below values only if you decide not to use DHCP
//byte ip[] = { 192, 168, 1, 62}; //put your devices local ip here
//byte dns1234[] = { 8, 8, 8, 8}; // google dns
//byte gateway[] = { 192, 168, 1, 254}; // local network gateway IP

bool quickstart = true; //change to false if you want to use registeres organisation
//char servername[]="quickstart.messaging.internetofthings.ibmcloud.com"; //change to suit "organisationID".messaging.internetofthings.ibmcloud.com
char servername[]="184.172.124.189"; //IP of your organisation ping site above to get IP ("windows use command prompt")
char clientStr[] = "d:quickstart:arduino:f0ae0c9f8414"; // change to suit d:organisationID:deviceType:deviceName
char token[] = ""; // insert your device token

char topicStr[] = "iot-2/evt/status/fmt/json"; // topic to which your device publishes messages the part you can chance is "status"

int SerialMessageLength = 170;
char data[170] = {}; //serial buffer

// Callback function header
void callback(char* topic, byte* payload, unsigned int length);

EthernetClient ethClient;
PubSubClient client(servername, 1883, callback, ethClient);

void callback(char* topic, byte* payload, unsigned int length) {
  //Serial.print("Message arrived [");
  //Serial.print(topic);
  //Serial.print("] ");
  Serial.print("\n");
  int msgLen = 0;
  for (int i=8;i<length-2;i++) { //reading only the command from the payload
    
    if(i != 12){
      Serial.print((char)payload[i]);
    }else{ //12th character cause we have deviceID 3bytes long and 4th byte is command
      if((char)payload[i-1] == '4' || (char)payload[i-1] == '3'){
        if(payload[i] == 126){
          Serial.print(';');
        }else{
          Serial.print((char)(payload[i]-33));
        }  
      }else{
        Serial.print((char)payload[i]);
      }
    }
    msgLen++;
  }
  
  while(msgLen < 7){
    msgLen++;
    Serial.print('x');
  }
  Serial.print("\r");

}

void reconnect()
{
  //Serial.println(clientStr);
  
  while (!client.connected()) {
    Serial.print(F("Reconnecting client ... \r"));
    if(!quickstart){
      client.connect(clientStr, "use-token-auth", token);
    }else{
      client.connect(clientStr);
    }
  }
  
  if(!quickstart){
      //subscribe to topic
      //client.subscribe("iot-2/cmd/cmd1/fmt/json"); //subscribe to one topic  DO NOT USE WITH QUICKSTART
      client.subscribe("iot-2/cmd/+/fmt/json");    //subscribe to all topics DO NOT USE WITH QUICKSTART
  }
  
  Serial.print(F("Connected!\r"));
}

void setup()
{ 
  Serial.begin(38400);
  Serial.print("\nRDYxxxx\r");//FireFly module on Xbee changing pins so serial communication can be established even if the pins are swapped
  delay(5000);
  Serial.print("\nRDYxxxx\r");
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.print("FireFly Gateway");
  lcd.setCursor(0, 1);
  lcd.print("firefly-iot.com");
  LiquidCrystal lcd(9, 8, 5, 4, 3, 2);
  
  //Ethernet.begin(mac, ip, dns1234, gateway); // start ethernet without DHCP
  Ethernet.begin(mac); // use DCHP
  client.setCallback(callback);
}

void loop()
{
  int index = 0;
  bool cloudSend = false;

  while(true){
    
    if(!client.loop())
    {
      reconnect();
    }
    
     while(Serial.available() > 0){
      //reading serial from FF1502 BLE module
      if(index < SerialMessageLength){
        data[index] = Serial.read();
        //Serial.print(data[index]);
        if(index == 0){
          if(data[index] == '!'){
            data[index] = ' ';
            index++;
          }
        }else{
          if(data[index] == '?'){
            data[index] = ' \0';
            index = 0;
            cloudSend = true;
          }else if(data[index] == '!'){
            data[0] == ' ';
            index=1;
          }else{
            index++;
          }
        }
      }else{
        index = 0;
      }
      if(cloudSend == true){
        cloudSend = false;
        Serial.print(data);
        Serial.print("\r");
        
        if (client.publish(topicStr,data)){
          Serial.print(F("successfully sent\r"));
          data[0] = '\0';
          break;
        }else{
          Serial.print(F("unsuccessfully sent\r"));
          data[0] = '\0';
          break;
        }
      } 
    }
  }
 
  Serial.print(F("Disconnecting client ...\r"));
  client.disconnect();
  delay(2000);
}

