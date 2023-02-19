
#include "esphome.h"

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

#include <webp/demux.h>

//#define TIDBYT
//#define ADAFRUIT_FEATHER_WING

#define TOPIC_PREFIX "plm"

#define BLACK           {0x00,0x00,0x00}
#define WHITE           {0xFF,0xFF,0xFF}
#define BLUE            {0x00,0x00,0xFF}
#define YELLOW          {0xFF,0xFF,0x00}
#define GREEN           {0x00,0xFF,0x00}
#define MAGENTA         {0xFF,0x00,0xFF}
#define RED             {0xFF,0x00,0x00}
#define CYAN            {0x00,0xFF,0xFF}

#define GRAY            {0x80,0x80,0x80}
#define LIGHT_GRAY      {0xC0,0xC0,0xC0}
#define PALE_GRAY       {0xE0,0xE0,0xE0}
#define DARK_GRAY       {0x40,0x40,0x40}

#define DARK_BLUE       {0x00,0x00,0x80}
#define DARK_GREEN      {0x00,0x80,0x00}
#define DARK_RED        {0x80,0x00,0x00}
#define LIGHT_BLUE      {0x80,0xC0,0xFF}
#define LIGHT_GREEN     {0x80,0xFF,0x80}
#define LIGHT_RED       {0xFF,0xC0,0xFF}
#define PINK            {0xFF,0xAF,0xAF}
#define BROWN           {0x60,0x30,0x00}
#define ORANGE          {0xFF,0x80,0x00}
#define PURPLE          {0xC0,0x00,0xFF}
#define LIME            {0x80,0xFF,0x00}

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

char hostName[20];
char applet_topic[22];
char applet_rts_topic[26];
char brightness_topic[22];
char lastProgText[12];
char statusAppletPath[30];
char messageToPublish[13];

WebPData webp_data;

int currentMode = WELCOME;
int desiredBrightness = 20;
int currentBrightness = 100;
unsigned long bufferPos;
bool recv_length = false;
bool otaInProgress = false;
bool need_publish = true;

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

void displayMessage(const char *message, uint8_t brightness)
{
  int16_t xOne, yOne;
  uint16_t w, h;
  uint8_t targetBrightness = 0;

  dma_display.clearScreen();
  dma_display.setBrightness8(targetBrightness);

  delay(100);

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

  while (targetBrightness < brightness)
  {
    dma_display.setBrightness8(targetBrightness);
    targetBrightness++;
    delay(10);
  }

  delay(1500);

  while (targetBrightness > 0)
  {
    dma_display.setBrightness8(targetBrightness);
    targetBrightness--;
    delay(10);
  }

  dma_display.clearScreen();
  
  delay(500);

  dma_display.setBrightness8(brightness);
}

class SmartMatrixComponent : public Component, public CustomMQTTDevice {
 public:
  float get_setup_priority() const override { return setup_priority::AFTER_WIFI; }

  void on_message(const std::string &payload) {
    const char *payloadstr = payload.c_str();
    size_t length = payload.length();
    if(strncmp(payloadstr,"START",5) == 0) {
        recv_length = false;
        bufferPos = 0;
        strcpy(messageToPublish, "OK");
        need_publish = true;
    } else if(strncmp(payloadstr,"PING",4) == 0) {
        strcpy(messageToPublish, "PONG");
        need_publish = true;
    } else if(!recv_length) {
        bufsize = atoi(payloadstr);
        tmpbuf = (uint8_t *) malloc(bufsize);
        recv_length = true;
        strcpy(messageToPublish, "OK");
        need_publish = true;
    } else {
        if(strncmp(payloadstr,"FINISH",6) == 0) {
            if (strncmp((const char*)tmpbuf, "RIFF", 4) == 0) {
                //Clear and reset all libwebp buffers.
                WebPDataClear(&webp_data);
                WebPDemuxReleaseIterator(&iter);
                WebPDemuxDelete(demux);

                //setup webp buffer and populate from temporary buffer
                webp_data.size = bufsize;
                webp_data.bytes = (uint8_t *) WebPMalloc(bufsize);
                if(webp_data.bytes == NULL) {
                    strcpy(messageToPublish, "DECODE_ERROR");
                    need_publish = true;
                } else {
                    memcpy((void *)webp_data.bytes, tmpbuf, bufsize);

                    //set display flags!
                    newapplet = true;
                    currentMode = APPLET;
                    strcpy(messageToPublish, "PUSHED");
                    need_publish = true;
                }
                free(tmpbuf);
            } else {
                strcpy(messageToPublish, "DECODE_ERROR");
                need_publish = true;
            }
            bufferPos = 0;
            recv_length = false;
        } else {
            memcpy((void *)(tmpbuf+bufferPos), payloadstr, length);
            bufferPos += length;
            strcpy(messageToPublish, "OK");
            need_publish = true;
        }
    }
  }

  void setup() override {
    esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
    sprintf(macFull, "%02X%02X%02X", baseMac[3], baseMac[4], baseMac[5]);

    Serial.println("smart-matrix: id: " + String(macFull));

    snprintf_P(applet_topic, 22, PSTR("%s/%s/rx"), TOPIC_PREFIX, macFull);
    snprintf_P(applet_rts_topic, 26, PSTR("%s/%s/tx"), TOPIC_PREFIX, macFull);


    Serial.println("smart-matrix: setting up display...");
    Serial.println("smart-matrix: topic: " + String(applet_topic));
    Serial.println("smart-matrix: rts topic: " + String(applet_rts_topic));

    dma_display.begin();
    dma_display.clearScreen();

    subscribe(applet_topic, &SmartMatrixComponent::on_message);

    strcpy(messageToPublish, "DEVICE_BOOT");
    need_publish = true;

    displayMessage("Hello", currentBrightness);
  }

  void loop() override {
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
                    if(millis() - last_frame_time > last_frame_duration) {
                        if(WebPDemuxGetFrame(demux, current_frame, &iter)) {
                            if(WebPDecodeRGBAInto(iter.fragment.bytes, iter.fragment.size, tempPixelBuffer, iter.width * iter.height * 4, iter.width * 4) != NULL) {
                                int px = 0;
                                for(int y = iter.y_offset; y < (iter.y_offset + iter.height); y++) {
                                    for(int x = iter.x_offset; x < (iter.x_offset + iter.width); x++) {
                                        //go pixel by pixel.
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
                    if(WebPDecodeRGBInto(webp_data.bytes, webp_data.size, tempPixelBuffer, MATRIX_HEIGHT * MATRIX_WIDTH * 3, MATRIX_WIDTH * 3) != NULL) {
                        
                        for(int y = 0; y < MATRIX_HEIGHT; y++) {
                            for(int x = 0; x < MATRIX_WIDTH; x++) {
                                int pixBitStart = ((y*MATRIX_WIDTH)+x)*3;
                                dma_display.writePixel(x,y, dma_display.color565(tempPixelBuffer[pixBitStart],tempPixelBuffer[pixBitStart+1],tempPixelBuffer[pixBitStart+2]));
                            }
                        }

                        currentMode = NONE;
                    }
                }
            }
        }
  }
};
