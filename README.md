# EPD
A library to control waveshare 1.54 inch e-paper display.

# Targets
- [200x200, 1.54inch E-Ink display module](https://www.waveshare.com/product/modules/oleds-lcds/e-paper/1.54inch-e-paper-module.htm)
- [200x200, 1.54inch E-Ink display module, three-color](https://www.waveshare.com/product/modules/oleds-lcds/e-paper/1.54inch-e-paper-module-b.htm)
- [152x152, 1.54inch E-Ink display module, yellow/black/white three-color](https://www.waveshare.com/product/modules/oleds-lcds/e-paper/1.54inch-e-paper-module-c.htm)

# Usage
## Connection
### Default

```
EPD1in54 epd; // default reset: 8, dc: 9, cs: 10, busy: 7
// EPD1in54B epd; // for red
// EPD1in54C epd; // for yellow
```

EPD | Controller
--- | ---
VCC | 3V3
GND | GND
SDI | MOSI
SCLK | SCK
CS | 10
DC | 9
Reset | 8
Busy | 7

### ESP32
You can change pin assign for CS, DC, Reset and Busy like this.
```
EPD1in54 epd(33, 25, 26, 27); // reset, dc, cs, busy
// EPD1in54B epd(33, 25, 26, 27); // for red
// EPD1in54C epd(33, 25, 26, 27); // for yellow
```

EPD | Controller
--- | ---
VCC | 3V3
GND | GND
SDI | MOSI(ESP32: 23)
SCLK | SCK(ESP32: 18)
CS | 26
DC | 25
Reset | 33
Busy | 27

## Program
See [examples](./examples).

# License
MIT

# References
- [ESP32で1.54インチのePaperを動かす方法](http://asukiaaa.blogspot.com/2019/01/esp32154epaper.html)
- [電子ペーパーの実験](https://garretlab.web.fc2.com/arduino/esp32/lab/epd/index.html)
- [1.54inch e-Paper Module](https://www.waveshare.com/wiki/1.54inch_e-Paper_Module)
- [1.54inch e-Paper Module (B)](https://www.waveshare.com/wiki/1.54inch_e-Paper_Module_(B))
- [1.54inch e-Paper Module (C)](https://www.waveshare.com/wiki/1.54inch_e-Paper_Module_(C))
