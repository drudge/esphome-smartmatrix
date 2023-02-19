
#pragma once
#include "esphome.h"

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

#include <webp/demux.h>

#include <Fonts/TomThumb.h>

//#define TIDBYT
//#define ADAFRUIT_FEATHER_WING

#define TOPIC_PREFIX "plm"

const int WELCOME = 1;
const int APPLET = 2;
const int NONE = 0;

#ifdef TIDBYT

// Change these to whatever suits
#define R1_PIN 21
#define G1_PIN 2
#define B1_PIN 22
#define R2_PIN 23
#define G2_PIN 4
#define B2_PIN 27
#define A_PIN 26
#define B_PIN 5
#define C_PIN 25
#define D_PIN 18
#define E_PIN -1 // required for 1/32 scan panels, like 64x64px. Any available pin would do, i.e. IO32
#define LAT_PIN 19
#define OE_PIN 32
#define CLK_PIN 33

HUB75_I2S_CFG::i2s_pins _pins={R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN};

HUB75_I2S_CFG mxconfig(
	64, // Module width
	32, // Module height
	1, // chain length
	_pins // pin mapping
);
MatrixPanel_I2S_DMA dma_display = MatrixPanel_I2S_DMA(mxconfig);
#elseif ADAFRUIT_FEATHER_WING
#define R1_PIN 6
#define G1_PIN 5
#define B1_PIN 9
#define R2_PIN 11
#define G2_PIN 10
#define B2_PIN 12
#define A_PIN 8
#define B_PIN 14
#define C_PIN 15
#define D_PIN 16
#define E_PIN -1
#define CLK_PIN 13
#define LAT_PIN 38
#define OE_PIN 39

HUB75_I2S_CFG::i2s_pins _pins={R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN};

HUB75_I2S_CFG mxconfig(
	64, // Module width
	32, // Module height
	1, // chain length
	_pins // pin mapping
);
MatrixPanel_I2S_DMA dma_display = MatrixPanel_I2S_DMA(mxconfig);
#else
MatrixPanel_I2S_DMA dma_display = MatrixPanel_I2S_DMA();
#endif

boolean newapplet = false;

char applet_topic[22];
char applet_rts_topic[26];
char messageToPublish[13];

WebPData webp_data;

int currentMode = WELCOME;
int currentBrightness = 100;
unsigned long bufferPos;
bool recv_length = false;
bool need_publish = true;
bool need_subscribe = true;
bool display_initialized = false;

uint8_t *tmpbuf;
uint8_t tempPixelBuffer[MATRIX_HEIGHT * MATRIX_WIDTH * 4];
unsigned long bufsize;

WebPDemuxer* demux;
WebPIterator iter;
uint32_t webp_flags;
uint32_t current_frame = 1;
uint32_t frame_count;

uint8_t baseMac[6];
char macFull[6];

unsigned long last_frame_duration = 0;
unsigned long last_frame_time = 0;
unsigned long last_check_tsl_time = 0;
unsigned long last_adjust_brightness_time = 0;

void showReady(const char *message, const char *id)
{
  int16_t xOne, yOne, xOneT, yOneT;
  uint16_t w, h, wT, hT;

  dma_display.clearScreen();

  dma_display.setCursor(0, 0);
  dma_display.setFont(NULL);
  dma_display.setTextSize(1);
  dma_display.setTextWrap(false);
  dma_display.setTextColor(dma_display.color565(255, 255, 255));

  dma_display.getTextBounds(message, 0, 0, &xOne, &yOne, &w, &h);

  int xPosition = dma_display.width() / 2 - w / 2 + 1;
  int yPosition = dma_display.height() / 2 - h / 2 - 4;

  dma_display.setCursor(xPosition, yPosition);
  dma_display.print(message);

  dma_display.getTextBounds(id, 0, 0, &xOneT, &yOneT, &wT, &hT);

  int messagesXPosition = dma_display.width() / 2 - (wT / 2) + 1;
  int messagesYPosition = yPosition + h + 4;

  dma_display.setCursor(messagesXPosition, messagesYPosition);
  dma_display.setFont(NULL);
  dma_display.setTextSize(1);
  dma_display.setTextColor(dma_display.color565(0, 161, 254));
  dma_display.setTextWrap(false);
  dma_display.print(id);
}

void sayHello() {
  if (!display_initialized) {
    return;
  }

  int16_t xOne, yOne;
  uint16_t w, h;
  uint8_t targetBrightness = 0;
  const char* message = "Hello";

  dma_display.clearScreen();
  dma_display.setBrightness8(targetBrightness);
  dma_display.setCursor(0, 0);
  dma_display.setFont(NULL);
  dma_display.setTextSize(1);
  dma_display.setTextWrap(false);
  dma_display.setTextColor(dma_display.color565(255, 255, 255));
  dma_display.getTextBounds(message, 0, 0, &xOne, &yOne, &w, &h);
  
  int xPosition = dma_display.width() / 2 - w / 2 + 1;
  int yPosition = dma_display.height() / 2 - h / 2 + 1;

  dma_display.setCursor(xPosition, yPosition);
  dma_display.print(message);

  while (targetBrightness < currentBrightness) {
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

  dma_display.setBrightness8(currentBrightness);
}

void showConnecting(const char *message)
{
  if (!display_initialized) {
    return;
  }

  dma_display.clearScreen();

  int16_t xOne, yOne;
  uint16_t w, h;

  dma_display.setFont(&TomThumb);
  dma_display.setCursor(0, 0);
  dma_display.setTextSize(1);
  dma_display.setTextWrap(false);
  dma_display.setTextColor(dma_display.color565(255, 255, 255));
  dma_display.getTextBounds(message, 0, 0, &xOne, &yOne, &w, &h);
  
  int xPosition = dma_display.width() / 2 - w / 2 + 1;
  int yPosition = dma_display.height() / 2 - h / 2 + 6;

  dma_display.setCursor(xPosition, yPosition);
  dma_display.print(message);
}

void setBrightness(int brightness)
{
  if (brightness > 255) {
    brightness = 255;
  }
  else if (brightness < 0) {
    brightness = 0;
  }

  currentBrightness = brightness;
  if (display_initialized) {
    dma_display.setBrightness8(brightness);
  }
}

void changeBrightness(int delta) {
  int newBrightness = currentBrightness + delta;
  setBrightness(newBrightness);
}

class SmartMatrixBrightnessOutput : public Component, public LightOutput {
 public:
  LightTraits get_traits() override {
    auto traits = LightTraits();
    traits.set_supported_color_modes({ColorMode::BRIGHTNESS});
    return traits;
  }

  void write_state(LightState *state) override {
    int newBrightness = state->current_values.get_brightness() * 255;
    setBrightness(newBrightness);
  }
};

class SmartMatrixComponent : public Component, public CustomMQTTDevice {
 public:
  float get_setup_priority() const override { return setup_priority::PROCESSOR; }

  void on_message(const std::string &payload) {
    const char *payloadstr = payload.c_str();
    size_t length = payload.length();

    if (strncmp(payloadstr,"START",5) == 0) {
      recv_length = false;
      bufferPos = 0;
      publishMessage("OK");
    } else if (strncmp(payloadstr,"PING",4) == 0) {
      publishMessage("PONG");
    } else if (!recv_length) {
      bufsize = atoi(payloadstr);
      tmpbuf = (uint8_t *) malloc(bufsize);
      recv_length = true;
      publishMessage("OK");
    } else {
        if (strncmp(payloadstr,"FINISH",6) == 0) {
          if (strncmp((const char*)tmpbuf, "RIFF", 4) == 0) {
            //Clear and reset all libwebp buffers.
            WebPDataClear(&webp_data);
            WebPDemuxReleaseIterator(&iter);
            WebPDemuxDelete(demux);

            //setup webp buffer and populate from temporary buffer
            webp_data.size = bufsize;
            webp_data.bytes = (uint8_t *) WebPMalloc(bufsize);

            if (webp_data.bytes == NULL) {
              publishMessage("DECODE_ERROR");
            } else {
              memcpy((void *)webp_data.bytes, tmpbuf, bufsize);

              //set display flags!
              newapplet = true;
              currentMode = APPLET;
              publishMessage("PUSHED");
            }
            free(tmpbuf);
          } else {
            publishMessage("DECODE_ERROR");
          }
          bufferPos = 0;
          recv_length = false;
        } else {
            memcpy((void *)(tmpbuf+bufferPos), payloadstr, length);
            bufferPos += length;
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
    esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
    sprintf(macFull, "%02X%02X%02X", baseMac[3], baseMac[4], baseMac[5]);

    Serial.println("smart-matrix: id: " + String(macFull));

    snprintf_P(applet_topic, 22, PSTR("%s/%s/rx"), TOPIC_PREFIX, macFull);
    snprintf_P(applet_rts_topic, 26, PSTR("%s/%s/tx"), TOPIC_PREFIX, macFull);

    if (!display_initialized) {
      dma_display.begin();
      dma_display.clearScreen();
      dma_display.setBrightness8(currentBrightness);

      display_initialized = true;

      sayHello();
    }

    if (is_connected()) {
      attachToServer();
    }
  }

  void loop() override {
    if (!is_connected()) {
      showConnecting("Connecting.  ");
      delay(400);
      showConnecting("Connecting.. ");
      delay(400);
      showConnecting("Connecting...");
      delay(400);
    } else {
      if (need_subscribe) {
        attachToServer();
      }

      if (need_publish) {
        publish(applet_rts_topic, messageToPublish);
        need_publish = false;
      }

      if (currentMode == APPLET) {
        if (newapplet) {
          demux = WebPDemux(&webp_data);
          frame_count = WebPDemuxGetI(demux, WEBP_FF_FRAME_COUNT);
          webp_flags = WebPDemuxGetI(demux, WEBP_FF_FORMAT_FLAGS);

          newapplet = false;
          current_frame = 1;
        } else {
          if (webp_flags & ANIMATION_FLAG) {
            if (millis() - last_frame_time > last_frame_duration) {
              if (WebPDemuxGetFrame(demux, current_frame, &iter)) {
                if (WebPDecodeRGBAInto(iter.fragment.bytes, iter.fragment.size, tempPixelBuffer, iter.width * iter.height * 4, iter.width * 4) != NULL) {
                  int px = 0;
                  for (int y = iter.y_offset; y < (iter.y_offset + iter.height); y++) {
                    for(int x = iter.x_offset; x < (iter.x_offset + iter.width); x++) {
                      int pixelOffsetCF = ((y*MATRIX_WIDTH)+x)*3;
                      int pixelOffsetFT = px*4;
                      int alphaValue = tempPixelBuffer[pixelOffsetFT+3];
                      
                      if(alphaValue == 255) {
                        dma_display.writePixel(x,y, dma_display.color565(tempPixelBuffer[pixelOffsetFT],tempPixelBuffer[pixelOffsetFT+1],tempPixelBuffer[pixelOffsetFT+2]));
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
                currentMode = NONE;
              }
            }
          } else {
            //Static WebP
            if (WebPDecodeRGBInto(webp_data.bytes, webp_data.size, tempPixelBuffer, dma_display.height() * dma_display.width() * 3, dma_display.width() * 3) != NULL) {
              for (int y = 0; y < dma_display.height(); y++) {
                for (int x = 0; x < dma_display.width(); x++) {
                  int pixBitStart = ((y*dma_display.width())+x)*3;
                  dma_display.writePixel(x,y, dma_display.color565(tempPixelBuffer[pixBitStart],tempPixelBuffer[pixBitStart+1],tempPixelBuffer[pixBitStart+2]));
                }
              }

              currentMode = NONE;
            }
          }
        }
      } else {
        showReady("Ready", macFull);
      }
    }
  }
};
