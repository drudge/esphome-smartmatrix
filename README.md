# esphome-smartmatrix

[SmartMatrix-ESP32](https://github.com/acvigue/SmartMatrix-ESP32) firmware for the internet connected display running on [ESPHome](https://esphome.io/index.html).

* Requires the [SmartMatrixServer](https://github.com/drudge/smart-matrix-server) to function.
* Built on the work by [acvique](https://github.com/acvigue/SmartMatrix-ESP32).
* Applets are written using [Pixlet](https://tidbyt.dev/docs/build/build-for-tidbyt)
* The matrix is powered by [ESP32-HUB75-MatrixPanel-DMA](https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-DMA) and assumes the [default pinouts](https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-DMA/blob/master/src/platforms/esp32/esp32-default-pins.hpp).
  * This firmware can support both Tidbyt hardware or the Adafruit RGB Feather Wing in addition to the default pinouts.
  * Uncomment the `#define TIDBYT` or `#define ADAFRUIT_FEATHER_WING` to adjust the pinout for those devices.
* The [ESP32 Trinity](https://esp32trinity.com/) by Brian Lough is a great board to use with SmartMatrix.
* This [64x32 panel from Waveshare](https://www.waveshare.com/rgb-matrix-p3-64x32.htm?amazon) works great with the library and is reasonably priced.


### ESP32 Trinity Example

The ESP32 Trinity board has two useful touchpads that we can tap into. Here is an example that uses them to control the brightness of the display. `setBrightness` and `changeBrightness` are helpers, but you can access the `dma_display` object to use the HUB75-MatrixPanel-DMA display object directly.


````yaml
esp32_touch:
  setup_mode: false

binary_sensor:
  - platform: esp32_touch
    id: t8_touchpad
    name: "T8 Touchpad"
    icon: "mdi:fingerprint"
    pin: GPIO33
    threshold: 600
    on_press:
      then:
        - lambda: changeBrightness(6);

  - platform: esp32_touch
    id: t9_touchpad
    name: "T9 Touchpad"
    icon: "mdi:fingerprint"
    pin: GPIO32
    threshold: 600
    on_press:
      then:
        - lambda: changeBrightness(-6);
```
