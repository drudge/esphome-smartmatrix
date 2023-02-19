# esphome-smartmatrix

[SmartMatrix-ESP32](https://github.com/acvigue/SmartMatrix-ESP32) firmware for the internet connected display running on [ESPhome](https://esphome.io/index.html).

* Requires the [SmartMatrixServer](https://github.com/drudge/smart-matrix-server) to function.
* Built on the work by [acvique](https://github.com/acvigue/SmartMatrix-ESP32).
* Applets are written using [Pixlet](https://tidbyt.dev/docs/build/build-for-tidbyt)
* The matrix is powered by [ESP32-HUB75-MatrixPanel-DMA](https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-DMA) and assumes the [default pinouts](https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-DMA/blob/master/src/platforms/esp32/esp32-default-pins.hpp).
* The [ESP32 Trinity](https://esp32trinity.com/) by Brian Lough is a great board to use with SmartMatrix.
* This [64x32 panel from Waveshare](https://www.waveshare.com/rgb-matrix-p3-64x32.htm?amazon) works great with the library and is reasonably priced.
