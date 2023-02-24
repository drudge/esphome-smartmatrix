
#pragma once

#include "esphome.h"
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <webp/demux.h>
#include <Fonts/TomThumb.h>

#define TOPIC_PREFIX "plm"

const int WELCOME = 1;
const int APPLET = 2;
const int NONE = 0;

#ifdef TIDBYT
#pragma message "Compiling for Tidbyt pins"
//HUB75_I2S_CFG::i2s_pins _pins = { 21, 2, 22, 23, 4, 27, 26, 5, 25, 18, -1, 19, 32, 16 }; // official pinout
HUB75_I2S_CFG::i2s_pins _pins = { 2, 22, 21, 4, 27, 23, 26, 5, 25, 18, -1, 19, 32, 33 }; // what actually works for me
#endif

#if !defined(TIDBYT) && defined(ADAFRUIT_FEATHER_WING)
#pragma message "Compiling for Adafruit RGB Matrix Feather Wing pins"
HUB75_I2S_CFG::i2s_pins _pins = { 6, 5, 9, 11, 10, 12, 8, 14, 15, 16, -1, 38, 39, 13 };
#endif

#if defined(TIDBYT) || defined(ADAFRUIT_FEATHER_WING)
HUB75_I2S_CFG matrix_config(MATRIX_WIDTH, MATRIX_HEIGHT, CHAIN_LENGTH, _pins);
MatrixPanel_I2S_DMA dma_display = MatrixPanel_I2S_DMA(matrix_config);
#else
#pragma message "Compiling with default pins"
MatrixPanel_I2S_DMA dma_display = MatrixPanel_I2S_DMA();
#endif

char applet_topic[22];
char applet_rts_topic[26];
char messageToPublish[13];

WebPData webp_data;

int current_mode = WELCOME;
int current_brightness = 100;
unsigned long buffer_position;
bool has_received_size = false;
bool need_publish = true;
bool need_subscribe = true;
bool display_initialized = false;
bool has_new_applet = false;
bool is_on = true;

uint8_t *buffer;
uint8_t pixel_buffer[MATRIX_HEIGHT * MATRIX_WIDTH * 4];
unsigned long buffer_size;

WebPDemuxer* demux;
WebPIterator iter;
uint32_t webp_flags;
uint32_t current_frame = 1;
uint32_t frame_count;

uint8_t mac_address[6];
char config_identifier[6];

unsigned long last_frame_duration = 0;
unsigned long last_frame_time = 0;

void showReady(const char *message, const char *id) {
  if (!display_initialized) {
    return;
  }

  int16_t x, y;
  uint16_t w, h;

  dma_display.clearScreen();

  dma_display.setCursor(0, 0);
  dma_display.setFont(NULL);
  dma_display.setTextSize(1);
  dma_display.setTextWrap(false);
  dma_display.setTextColor(dma_display.color565(255, 255, 255));

  dma_display.getTextBounds(message, 0, 0, &x, &y, &w, &h);

  int x_position = dma_display.width() / 2 - w / 2 + 1;
  int y_position = dma_display.height() / 2 - h / 2 - 4;

  dma_display.setCursor(x_position, y_position);
  dma_display.print(message);
  dma_display.getTextBounds(id, 0, 0, &x, &y, &w, &h);

  dma_display.setCursor(dma_display.width() / 2 - (w / 2) + 1, y_position + h + 4);
  dma_display.setFont(NULL);
  dma_display.setTextSize(1);
  dma_display.setTextColor(dma_display.color565(0, 161, 254));
  dma_display.setTextWrap(false);
  dma_display.print(id);
}

void sayHello(char* message = "Hello") {
  if (!display_initialized) {
    return;
  }

  int16_t xOne, yOne;
  uint16_t w, h;
  uint8_t targetBrightness = 0;

  dma_display.clearScreen();
  dma_display.setBrightness8(targetBrightness);
  dma_display.setCursor(0, 0);
  dma_display.setFont(NULL);
  dma_display.setTextSize(1);
  dma_display.setTextWrap(false);
  dma_display.setTextColor(dma_display.color565(255, 255, 255));
  dma_display.getTextBounds(message, 0, 0, &xOne, &yOne, &w, &h);
  
  int x_position = dma_display.width() / 2 - w / 2 + 1;
  int y_position = dma_display.height() / 2 - h / 2 + 1;

  dma_display.setCursor(x_position, y_position);
  dma_display.print(message);

  while (targetBrightness < current_brightness) {
    dma_display.setBrightness8(targetBrightness);
    targetBrightness++;
    delay(10);
  }

  delay(1500);

  while (targetBrightness > 0) {
    dma_display.setBrightness8(targetBrightness);
    targetBrightness--;
    delay(10);
  }
  
  delay(500);

  dma_display.setBrightness8(current_brightness);
}

void showConnecting(const char *message) {
  if (!display_initialized) {
    return;
  }

  dma_display.clearScreen();

  int16_t x, y;
  uint16_t w, h;

  dma_display.setFont(&TomThumb);
  dma_display.setCursor(0, 0);
  dma_display.setTextSize(1);
  dma_display.setTextWrap(false);
  dma_display.setTextColor(dma_display.color565(255, 255, 255));
  dma_display.getTextBounds(message, 0, 0, &x, &y, &w, &h);
  
  int x_position = dma_display.width() / 2 - w / 2 + 1;
  int y_position = dma_display.height() / 2 - h / 2 + 6;

  dma_display.setCursor(x_position, y_position);
  dma_display.print(message);
}

void setBrightness(int brightness) {
  if (!is_on) {
    dma_display.setBrightness8(0);
  }

  if (brightness > 255) {
    brightness = 255;
  }
  else if (brightness < 0) {
    brightness = 0;
  }

  current_brightness = brightness;
  if (display_initialized) {
    dma_display.setBrightness8(current_brightness);
  }
}

void changeBrightness(int delta) {
  int new_brightness = current_brightness + delta;
  setBrightness(new_brightness);
}

class SmartMatrixBrightnessOutput : public Component, public LightOutput {
 public:
  LightTraits get_traits() override {
    auto traits = LightTraits();
    traits.set_supported_color_modes({ColorMode::BRIGHTNESS});
    return traits;
  }

  void write_state(LightState *state) override {
    is_on = state->current_values.is_on();
    int new_brightness = state->current_values.get_brightness() * 255;

    setBrightness(new_brightness);
  }
};

class SmartMatrixComponent : public Component, public CustomMQTTDevice {
 public:
  float get_setup_priority() const override { return setup_priority::PROCESSOR; }

  void on_message(const std::string &payload) {
    const char *payloadstr = payload.c_str();
    size_t length = payload.length();

    if (strncmp(payloadstr,"START",5) == 0) {
      has_received_size = false;
      buffer_position = 0;
      publishMessage("OK");
    } else if (strncmp(payloadstr,"PING",4) == 0) {
      publishMessage("PONG");
    } else if (!has_received_size) {
      buffer_size = atoi(payloadstr);
      buffer = (uint8_t *) malloc(buffer_size);
      has_received_size = true;
      publishMessage("OK");
    } else {
        if (strncmp(payloadstr,"FINISH",6) == 0) {
          if (strncmp((const char*)buffer, "RIFF", 4) == 0) {
            //Clear and reset all libwebp buffers.
            WebPDataClear(&webp_data);
            WebPDemuxReleaseIterator(&iter);
            WebPDemuxDelete(demux);

            //setup webp buffer and populate from temporary buffer
            webp_data.size = buffer_size;
            webp_data.bytes = (uint8_t *) WebPMalloc(buffer_size);

            if (webp_data.bytes == NULL) {
              publishMessage("DECODE_ERROR");
            } else {
              memcpy((void *)webp_data.bytes, buffer, buffer_size);

              //set display flags!
              has_new_applet = true;
              current_mode = APPLET;
              publishMessage("PUSHED");
            }
            free(buffer);
          } else {
            publishMessage("DECODE_ERROR");
          }
          buffer_position = 0;
          has_received_size = false;
        } else {
            memcpy((void *)(buffer+buffer_position), payloadstr, length);
            buffer_position += length;
            publishMessage("OK");
        }
    }
  }

  void publishMessage(std::string message) {
    strcpy(messageToPublish, message.c_str());
    need_publish = true;
  }

  void attachToServer() {
    if (need_subscribe) {
      subscribe(applet_topic, &SmartMatrixComponent::on_message);
      need_subscribe = false;
    }
    publishMessage("DEVICE_BOOT");
  }

  void setup() override {
    esp_read_mac(mac_address, ESP_MAC_WIFI_STA);
    sprintf(config_identifier, "%02X%02X%02X", mac_address[3], mac_address[4], mac_address[5]);
    snprintf_P(applet_topic, 22, PSTR("%s/%s/rx"), TOPIC_PREFIX, config_identifier);
    snprintf_P(applet_rts_topic, 26, PSTR("%s/%s/tx"), TOPIC_PREFIX, config_identifier);

    if (!display_initialized) {
      dma_display.begin();
      dma_display.clearScreen();
      setBrightness(current_brightness);

      display_initialized = true;

      sayHello();
    }

    if (is_connected()) {
      attachToServer();
    }
  }

  void loop() override {
    if (!esphome::wifi::global_wifi_component->has_sta()) {
      sayHello("Join WiFi");
    } else if (!is_connected()) {
      showConnecting("Connecting.  ");
      delay(400);
      showConnecting("Connecting.. ");
      delay(400);
      showConnecting("Connecting...");
      delay(400);
    } else {
      if (!is_on) {
        setBrightness(0);
      }

      if (need_subscribe) {
        attachToServer();
      }

      if (need_publish) {
        publish(applet_rts_topic, messageToPublish);
        need_publish = false;
      }

      if (current_mode == APPLET) {
        if (has_new_applet) {
          demux = WebPDemux(&webp_data);
          frame_count = WebPDemuxGetI(demux, WEBP_FF_FRAME_COUNT);
          webp_flags = WebPDemuxGetI(demux, WEBP_FF_FORMAT_FLAGS);

          has_new_applet = false;
          current_frame = 1;
        } else {
          if (webp_flags & ANIMATION_FLAG) {
            if (millis() - last_frame_time > last_frame_duration) {
              if (WebPDemuxGetFrame(demux, current_frame, &iter)) {
                if (WebPDecodeRGBAInto(iter.fragment.bytes, iter.fragment.size, pixel_buffer, iter.width * iter.height * 4, iter.width * 4) != NULL) {
                  int px = 0;
                  for (int y = iter.y_offset; y < (iter.y_offset + iter.height); y++) {
                    for(int x = iter.x_offset; x < (iter.x_offset + iter.width); x++) {
                      int pixelOffsetCF = ((y*MATRIX_WIDTH)+x)*3;
                      int pixelOffsetFT = px*4;
                      int alphaValue = pixel_buffer[pixelOffsetFT+3];
                      
                      if(alphaValue == 255) {
                        dma_display.writePixel(x,y, dma_display.color565(pixel_buffer[pixelOffsetFT],pixel_buffer[pixelOffsetFT+1],pixel_buffer[pixelOffsetFT+2]));
                      }
                      
                      px++;
                    }
                  }

                  last_frame_time = millis();
                  last_frame_duration = iter.duration;
                  current_frame++;
                  if(current_frame > frame_count) {
                    current_frame = 1;
                  }
              }
              } else {
                current_mode = NONE;
              }
            }
          } else {
            //Static WebP
            if (WebPDecodeRGBInto(webp_data.bytes, webp_data.size, pixel_buffer, dma_display.height() * dma_display.width() * 3, dma_display.width() * 3) != NULL) {
              for (int y = 0; y < dma_display.height(); y++) {
                for (int x = 0; x < dma_display.width(); x++) {
                  int pixBitStart = ((y*dma_display.width())+x)*3;
                  dma_display.writePixel(x,y, dma_display.color565(pixel_buffer[pixBitStart],pixel_buffer[pixBitStart+1],pixel_buffer[pixBitStart+2]));
                }
              }

              current_mode = NONE;
            }
          }
        }
      } else if (current_mode == WELCOME) {
        showReady("Ready", config_identifier);
      }
    }
  }
};
