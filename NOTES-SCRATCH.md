# Notes
* using esp-idf v5.5.1 for testin
## Wireshark
* filter for finding these adverts by mac is `btle.advertising_address == xx:xx:xx:xx:xx || btle.scanning_address == xx:xx:xx:xx:xx`
* filter for narrowing to without mac is `(btcommon.eir_ad.entry.type == 0x01) && (btle.advertising_header == 0x2506)`
## Voltage reading
![Wiring SDC to bat voltage to measure with ADC0](assets/read-bat-voltage-schematic.png)
1. 2x 200k resistor
    1. bat\+ -> adc0
    2. bat\- -> adc0
2. [adc oneshot read esp-idf](https://github.com/espressif/esp-idf/tree/v5.5.1/examples/peripherals/adc/oneshot_read)
3. [battery usage Xiao C6](https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/#battery-usage)
4. [battery usage Xiao C3](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/#battery-usage)
* max usable interval 4000ms seems like
    * 2000ms seems moar stable? idk
# Scratch
burst-60s-200ms-10ms-3b-nousb-ant-f68889e-