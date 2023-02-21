# esphome-smartmatrix

![219930526-6cdd91fe-4f57-40fa-a8c9-399ac05e1864](https://user-images.githubusercontent.com/72890/219930692-824a2063-1788-4628-8d7f-fdbc240b5191.png)

[SmartMatrix](https://github.com/acvigue/SmartMatrix-ESP32) firmware for the internet connected display using [ESPHome](https://esphome.io/index.html).

* Requires the [SmartMatrix server](https://github.com/drudge/smart-matrix-server) to function.
* Applets are written in [Starlark](https://github.com/google/starlark-go/blob/master/doc/spec.md) using the [Pixlet](https://tidbyt.dev/docs/build/build-for-tidbyt) runtime.
* The matrix display is powered by [ESP32-HUB75-MatrixPanel-DMA](https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-DMA).
* The [ESP32 Trinity](https://esp32trinity.com/) by Brian Lough is a great board to use with SmartMatrix.
* This [64x32 panel from Waveshare](https://www.waveshare.com/rgb-matrix-p3-64x32.htm?amazon) works great with the library and is reasonably priced.
* Based on work by [acvique](https://github.com/acvigue/SmartMatrix-ESP32).

# Pinouts

![image](https://user-images.githubusercontent.com/72890/219923621-937a8f2e-1026-4c38-bb05-2691427aea0b.png)

|           | Default Pin |       Tidbyt    |    Adafruit Feather Wing  |
|----------:|:-----------:|:---------------:|:-------------------------:|
|     **R1**    |    GPIO25   |    GPIO21   |          GPIO06       |
|     **G1**    |    GPIO26   |    GPIO02   |          GPIO05       |
|     **B1**    |    GPIO27   |    GPIO22   |          GPIO09       |
|     **R2**    |    GPIO14   |    GPIO23   |          GPIO11       |
|     **G2**    |    GPIO12   |    GPIO04   |          GPIO10       |
|     **B2**    |    GPIO13   |    GPIO27   |          GPIO12       |
|     **A**     |    GPIO23   |    GPIO26   |          GPIO08       |
|     **B**     |    GPIO19   |    GPIO05   |          GPIO14       |
|     **C**     |    GPIO05   |    GPIO25   |          GPIO15       |
|     **D**     |    GPIO17   |    GPIO18   |          GPIO16       |
|     **E**     |    -1       |    -1       |          -1           |
|     **LAT**   |    GPIO04   |    GPIO19   |          GPIO38       |
|     **OE**    |    GPIO15   |    GPIO32   |          GPIO39       |
|     **CLK**   |    GPIO16   |    GPIO33   |          GPIO13       |


This firmware can support both Tidbyt hardware or the Adafruit RGB Feather Wing in addition to the default pinouts.

To switch, uncomment the `#define TIDBYT` or `#define ADAFRUIT_FEATHER_WING` accordingly.

# Home Assistant

Tha Matrix Display will appear as a light in Home Assistant. You can turn the display on or off and adjust the brightness. Brightness levels are saved and will restore on device boot.

### ESP32 Trinity Example

The ESP32 Trinity board has two useful touchpads that we can tap into. Here is an example that uses them to control the brightness of the display:

```yaml
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

Note: `setBrightness` and `changeBrightness` are helpers, but you can access the `dma_display` variable to use the HUB75-MatrixPanel-DMA display object directly in your lambdas.
