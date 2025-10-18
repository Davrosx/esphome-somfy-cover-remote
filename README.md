# ESPHome Somfy cover remote
This is an external component for [ESPHOME](https://esphome.io/), to control Somfy RTS covers from for example [Home Assisant](https://www.home-assistant.io/).

## Required hardware
- ESP32
- CC1101 RF module

## Setup
Use the following ESPHome yaml as a base for your Somfy controller. Add one ore more covers, depending on your needs.

```
esphome:
  name: somfycontroller

external_components:
  - source: github://Davrosx/esphome-cc1101@main
    components: [ cc1101 ]
  - source: github://Davrosx/esphome-somfy-cover-remote@main
    components: [ somfy_cover ]

esp32:
  board: esp32dev
  framework:
    type: arduino

# Enable logging
logger:

# Enable Home Assistant API
api:

ota:
  platform: esphome
  password: !secret ota_password

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password

  Enable fallback hotspot (captive portal) in case wifi connection fails
  ap:
    ssid: "Somfycontroller Fallback Hotspot"
    password: !secret fallback_hotspot

button:
  - platform: template
    id: "program_livingroom"
    name: "Program livingroom"
  - platform: template
    id: "program_kitchen"
    name: "Program kitchen"
  - platform: template
    id: "program_study"
    name: "Program study"
  - platform: template
    id: "program_bathroom"
    name: "Program bathroom"

# For the WT32-ETH01 ESP module, you can use these pins
cc1101:
  id: "cc1101_module"
  cc1101_frequency: 433.42
  mosi_pin: 12
  miso_pin: 39
  clk_pin: 14
  cs_pin: 15
  cc1101_emitter_pin: 2

cover:
  - platform: somfy_cover
    id: "livingroom"
    name: "Livingroom cover"
    device_class: shutter
    open_duration: 18s
    close_duration: 17s
    remote_code: 0x6b2a03
    storage_key: "livingroom"
    prog_button: "program_livingroom"
    cc1101_module: "cc1101_module"
  
  - platform: somfy_cover
    id: "kitchen"
    name: "Kitchen cover"
    device_class: shutter
    open_duration: 26s
    close_duration: 25s
    remote_code: 0x0bf93b
    storage_key: "kitchen"
    prog_button: "program_kitchen"
    cc1101_module: "cc1101_module"
  
  - platform: somfy_cover
    id: "study"
    name: "Study cover"
    device_class: shutter
    open_duration: 18s
    close_duration: 17s
    remote_code: 0x09a1c3
    storage_key: "study"
    prog_button: "program_study"
    cc1101_module: "cc1101_module"

  - platform: somfy_cover
    id: "bathroom"
    name: "Bathroom Cover"
    device_class: shutter
    open_duration: 18s
    close_duration: 17s
    remote_code: 0x449677
    storage_key: "bathroom"
    prog_button: "program_bathroom"
    cc1101_module: "cc1101_module"
    repeat_command_count: 1

```

## Generate remote code 
The remote code is a three byte hex code.
For example, use the website: https://www.browserling.com/tools/random-hex  
Set to 6 digits and add `0x` in front of the generated hex number.

## Pair the cover
Put your cover in program mode with another remote, then use the `Program x` button to pair with the ESP. From then on the cover should respond to the ESPHome Somfy controller. You can also connect multiple covers by pairing then one by one with the same `Program x` button.

## Repeating command setting
The *Somfy_Remote_Lib* library defaults to sending a command four times. Some devices do not handle this well and should only reveive the command one time. For these devices the optional parameter `repeat_command_count` can be set in the yaml for the cover.

## Credits
Project originally used the ESPHome custom component from [evgeni](https://github.com/evgeni/esphome-configs/) after finding his article [Controlling Somfy roller shutters using an ESP32 and ESPHome](https://www.die-welt.net/2021/06/controlling-somfy-roller-shutters-using-an-esp32-and-esphome/).

Project needed to be able to specify the cover position by used the ESPHome [Time Based Cover](https://esphome.io/components/cover/time_based.html) component. Code was used from evgeni as as base and used the ESPHome Custom component option to implement this. Since the Custom component option is deprecated and removed, the project needed another solution. Therefore this external component was created.
