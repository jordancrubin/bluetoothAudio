/*
  VW Sapphire IX Antique Car Stereo Bluetooth Project
  Used in conjunction with external DAC and Mux to allow for
  Automatically switching between MONO AM and MONO Bluetooth
  With Voice affirming connect/disconnect notifications  
  https://www.youtube.com/c/jordanrubin6502
  2021 Jordan Rubin.
*/
#include <Arduino.h>
#include "audiodata.h"
#include "BluetoothA2DPSink.h"
#include "driver/i2s.h"
const int muxPin = 18;
const unsigned int bluetoothConn_raw_len = 43600;    //CONN
const unsigned int bluetoothDisconn_raw_len = 55730; //DISCONN
const i2s_port_t I2S_PORT = I2S_NUM_0;
const unsigned char *ConnRawFile=bluetoothConn_raw;       //CONN
const unsigned char *DisconnRawFile=bluetoothDisconn_raw; //DISCONN
BluetoothA2DPSink a2dp_sink;
esp_a2d_connection_state_t last_state;
unsigned const char* AudioData;
bool playmsg;

//------playAudio------//
void playAudio(bool type) {
  AudioData = ConnRawFile;
  int len = bluetoothConn_raw_len;
  if (type == 0) {
    AudioData = DisconnRawFile;
    len = bluetoothDisconn_raw_len;
  }
  i2s_set_sample_rates(I2S_PORT, 11025);
  uint32_t index = 0; 
  size_t BytesWritten;
  const unsigned char *Data;
  int rest;
  int byteSize = 4;
  digitalWrite(muxPin, HIGH);
  playmsg = 1;
  while (index < len) {
    rest = len - index;
    if (rest < 4){byteSize = rest;}
    Data=AudioData+index;
    i2s_write(I2S_PORT,Data,byteSize,&BytesWritten,portMAX_DELAY); 
    index+=4;
  }
  playmsg = 0;
  i2s_set_sample_rates(I2S_PORT, 44100);
}
//------connect_bt_callback------//
void connect_bt_callback() {
  playAudio(1);
}

//------disconnect_bt_callback------//
void disconnect_bt_callback() {
  playAudio(0);
  //ESP.restart();
}

//------setup------//
void setup() {
  Serial.begin(115200);
      i2s_pin_config_t my_pin_config = {
        .bck_io_num = 26,
        .ws_io_num = 25,
        .data_out_num = 23,
        .data_in_num = I2S_PIN_NO_CHANGE
    };
  pinMode(muxPin, OUTPUT);
  a2dp_sink.set_pin_config(my_pin_config);
  a2dp_sink.set_mono_downmix(1);
  a2dp_sink.set_on_connected2BT(connect_bt_callback);
  a2dp_sink.set_on_disconnected2BT(disconnect_bt_callback);
  a2dp_sink.start("RTECH VW Sapphire IX"); 
}

//------loop------//
void loop() {
  esp_a2d_audio_state_t audiostate = a2dp_sink.get_audio_state();
  Serial.println(audiostate);
  if ((audiostate == 2)||(playmsg == 1)){
    digitalWrite(muxPin, HIGH);
  }
  else {
    digitalWrite(muxPin, LOW);
  }
  delay(500);
}