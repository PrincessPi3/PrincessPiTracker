# Notes
* using esp-idf v5.5.1 for testin
## Wireshark
* filter for finding these adverts by mac is `btle.advertising_address == xx:xx:xx:xx:xx || btle.scanning_address == xx:xx:xx:xx:xx || btle.target_address == xx:xx:xx:xx:xx`
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
## test protocol
rebuildfull
erase flash
set setgings
flash
plug into usb-c meter
measure power usage over 10 minutes (600 seconds)
export to excel
    name like:
        (progress markers: b built, f flashed, m measuring, l logged, d done)
        bfmld burst-60s-200ms-10ms-3b-nousb-ant-109f09e-
        bfmld normal-10.230s-10ms-nousb-ant-109f09e-
        bfmld normal-10.230s-10ms-nousb-noant-109f09e-
        bfm burst-60s-200ms-10ms-3b-nousb-noant-109f09e-
    next
        burst-120s-40ms-0ms-4b-nousb-noant-2975c2c-
get values with (EXCEL paste into cell D1)
<table border="1">
  <tr>
    <th>avg V</th>
    <th>avg A</th>
    <th>avg W</th>
    <th>avg mW</th>
  </tr>
  <tr>
    <td>=AVERAGE(B1:B1000)</td>
    <td>=AVERAGE(C1:C1000)</td>
    <td>=D2*E2</td>
    <td>=F2*1000</td>
  </tr>
</table>