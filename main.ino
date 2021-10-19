/*
 * Example for how to use SinricPro TV device:
 * - setup a TV device
 * - handle request using callbacks
 *
 * If you encounter any issues:
 * - check the readme.md at https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md
 * - ensure all dependent libraries are installed
 *   - see https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md#arduinoide
 *   - see https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md#dependencies
 * - open serial monitor and check whats happening
 * - check full user documentation at https://sinricpro.github.io/esp8266-esp32-sdk
 * - visit https://github.com/sinricpro/esp8266-esp32-sdk/issues and check for existing issues or open a new one
 */

#include <Arduino.h>
#include <IRremoteESP8266.h> // IR library
#include <IRsend.h> // IR library
const uint16_t kIrLed = 27;  // GPIO pin to use. Recommended: 4 (D2).
IRsend irsend(kIrLed);  // Set the GPIO to be used to sending the message.

// Include the right wifi library
#ifdef ESP8266
       #include <ESP8266WiFi.h>
#endif
#ifdef ESP32
       #include <WiFi.h>
#endif

// Include Sinric Pro TV API
#include "SinricPro.h"
#include "SinricProTV.h"

#define WIFI_SSID         "YourWifiAccessPointName"
#define WIFI_PASS         "ExamplePassword1234"
#define APP_KEY           "Sinric Pro app key"      // Should look like "de0bxxxx-1x3x-4x3x-ax2x-5dabxxxxxxxx"
#define APP_SECRET        "Sinric Pro app secret"   // Should look like "5f36xxxx-x3x7-4x3x-xexe-e86724a9xxxx-4c4axxxx-3x3x-x5xe-x9x3-333d65xxxxxx"
#define TV_ID             "Sinric Pro device ID"    // Should look like "5dc1564130xxxxxxxxxxxxxx"
#define BAUD_RATE         115200

bool tvPowerState;
unsigned int tvVolume;
unsigned int tvChannel;
bool tvMuted;

// channelNames used to convert channelNumber into channelName
// please put in your TV channel names
// channel numbers starts counting from 0!
// so "ZDF" is channel 2
const char* channelNames[] = {
  "A/V",
  "Rai 1", 
  "Rai 2", 
  "Rai 3", 
  "Rete 4", 
  "Canale 5", 
  "Italia 1", 
  "La7", 
  "TV8", 
  "Nove", 
  "SUPER RTL", 
  "KiKA"
};

#define MAX_CHANNELS sizeof(channelNames) / sizeof(channelNames[0])  // Just to determine how many channels are in channelNames array

// Map channelNumbers used to convert channelName into channelNumber
// This map is initialized in "setupChannelNumbers()" function by using the "channelNames" array
std::map<String, unsigned int> channelNumbers;

void setupChannelNumbers() {
  for (unsigned int i=0; i < MAX_CHANNELS; i++) {
    channelNumbers[channelNames[i]] = i;
  }
}

// TV device callbacks

bool onAdjustVolume(const String &deviceId, int &volumeDelta, bool volumeDefault) {
  if (volumeDelta > 0) {irsend.sendSAMSUNG(0xE0E0E01F, 32);delay(10);irsend.sendSAMSUNG(0xE0E0E01F, 32);delay(10);irsend.sendSAMSUNG(0xE0E0E01F, 32);delay(10);irsend.sendSAMSUNG(0xE0E0E01F, 32);delay(10);irsend.sendSAMSUNG(0xE0E0E01F, 32);} //Insert IR for Volume Up here.
  if (volumeDelta < 0) {irsend.sendSAMSUNG(0xE0E0D02F, 32);delay(10);irsend.sendSAMSUNG(0xE0E0D02F, 32);delay(10);irsend.sendSAMSUNG(0xE0E0D02F, 32);delay(10);irsend.sendSAMSUNG(0xE0E0D02F, 32);delay(10);irsend.sendSAMSUNG(0xE0E0D02F, 32);} //Insert IR for Volume Down here.
  tvVolume += volumeDelta;  // calcualte new absolute volume
  Serial.printf("Volume changed about %i to %i\r\n", volumeDelta, tvVolume);
  volumeDelta = tvVolume; // return new absolute volume
  
  return true;
}

bool onChangeChannel(const String &deviceId, String &channel) {
  tvChannel = channelNumbers[channel]; // save new channelNumber in tvChannel variable
  Serial.printf("Change channel to \"%s\" (channel number %d)\r\n", channel.c_str(), tvChannel);
  if (channel == "Rai 2") {irsend.sendSAMSUNG(0xE0E0A05F);irsend.sendSAMSUNG(0xE0E016E9);} //Change IR sequence inside of {} for each channel.
  if (channel == "Rai 1") {irsend.sendSAMSUNG(0xE0E020DF);irsend.sendSAMSUNG(0xE0E016E9);}
  
  return true;
}

bool onChangeChannelNumber(const String& deviceId, int channelNumber, String& channelName) {
  tvChannel = channelNumber; // update tvChannel to new channel number
  if (tvChannel < 0) tvChannel = 0;
  if (tvChannel > MAX_CHANNELS-1) tvChannel = MAX_CHANNELS-1;

  channelName = channelNames[tvChannel]; // return the channelName

  Serial.printf("Change to channel to %d (channel name \"%s\")\r\n", tvChannel, channelName.c_str());
  return true;
}

bool onMediaControl(const String &deviceId, String &control) {
  Serial.printf("MediaControl: %s\r\n", control.c_str());
  if (control == "Play") {irsend.sendSAMSUNG(0xE0E0E21D);}           // do whatever you want to do here
  if (control == "Pause") {irsend.sendSAMSUNG(0xE0E052AD);}          // do whatever you want to do here
  if (control == "Stop") {irsend.sendSAMSUNG(0xE0E0629D);}           // do whatever you want to do here
//if (control == "StartOver") {irsend.sendSAMSUNG(0xFFFFFFFF);}      // do whatever you want to do here
//if (control == "Previous") {irsend.sendSAMSUNG(0xFFFFFFFF);}       // do whatever you want to do here
//if (control == "Next") {irsend.sendSAMSUNG(0xFFFFFFFF);}           // do whatever you want to do here
  if (control == "Rewind") {irsend.sendSAMSUNG(0xE0E0A25D);}         // do whatever you want to do here
//if (control == "FastForward") {irsend.sendSAMSUNG(0xFFFFFFFF);}    // do whatever you want to do here
  return true;
}

bool onMute(const String &deviceId, bool &mute) {
  Serial.printf("TV volume is %s\r\n", mute?"muted":"unmuted");
  tvMuted = mute; // set tvMuted state
  irsend.sendSAMSUNG(0xE0E0F00F);
  return true;
}

bool onPowerState(const String &deviceId, bool &state) {
  Serial.printf("TV turned %s\r\n", state?"on":"off");
  tvPowerState = state; // set powerState
  
  irsend.sendSAMSUNG(0xE0E040BF,32);
  return true;
}

bool onSelectInput(const String &deviceId, String &input) {
  Serial.printf("Input changed to %s\r\n", input.c_str());
  
  irsend.sendSAMSUNG(0xE0E0807F);
  return true;
}

bool onSetVolume(const String &deviceId, int &volume) {
  Serial.printf("Volume set to:  %i\r\n", volume);
  tvVolume = volume; // update tvVolume
  return true;
}

bool onSkipChannels(const String &deviceId, const int channelCount, String &channelName) {
  tvChannel += channelCount; // calculate new channel number
  if (tvChannel < 0) {tvChannel = 0;irsend.sendSAMSUNG(0xE0E0E01F);}
  if (tvChannel > MAX_CHANNELS-1) {tvChannel = MAX_CHANNELS-1;irsend.sendSAMSUNG(0xE0E0D02F);}
  channelName = String(channelNames[tvChannel]); // return channel name

  Serial.printf("Skip channel: %i (number: %i / name: \"%s\")\r\n", channelCount, tvChannel, channelName.c_str());

  return true;
}

// setup function for WiFi connection
void setupWiFi() {
  Serial.printf("\r\n[Wifi]: Connecting");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(250);
  }
  IPAddress localIP = WiFi.localIP();
  Serial.printf("connected!\r\n[WiFi]: IP-Address is %d.%d.%d.%d\r\n", localIP[0], localIP[1], localIP[2], localIP[3]);
}

// setup function for SinricPro
void setupSinricPro() {
  // add device to SinricPro
  SinricProTV& myTV = SinricPro[TV_ID];

  // set callback functions to device
  myTV.onAdjustVolume(onAdjustVolume);
  myTV.onChangeChannel(onChangeChannel);
  myTV.onChangeChannelNumber(onChangeChannelNumber);
  myTV.onMediaControl(onMediaControl);
  myTV.onMute(onMute);
  myTV.onPowerState(onPowerState);
  myTV.onSelectInput(onSelectInput);
  myTV.onSetVolume(onSetVolume);
  myTV.onSkipChannels(onSkipChannels);

  // setup SinricPro
  SinricPro.onConnected([](){ Serial.printf("Connected to SinricPro\r\n"); }); 
  SinricPro.onDisconnected([](){ Serial.printf("Disconnected from SinricPro\r\n"); });
  SinricPro.begin(APP_KEY, APP_SECRET);
}

// main setup function
void setup() {
  //pinMode(27, OUTPUT);
  
  Serial.begin(BAUD_RATE); Serial.printf("\r\n\r\n");
  Serial.printf("%d channels configured\r\n", MAX_CHANNELS);

  setupWiFi();
  setupSinricPro();
  setupChannelNumbers();
  irsend.begin(); // Start ir library
}

void loop() {
  SinricPro.handle();
}
