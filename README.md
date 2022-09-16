# RTS_Link

<img src="https://github.com/firedream89/RTS_Link/blob/main/final.jpg" width="300" height="500">

RTS link to home assistant with an arduino nano and transceiver RFM69HW

Based on SomfyRTS library :
https://github.com/etimou/SomfyRTS

just a small program to link RTS shutters with home assistant(RFLINK integration)

## first use :
RESET : before starting on home assistant, send this command with Serial monitor.
<br>
the shutters id starting at 10 and increment every the command PROG was send.

## On home assistant :
<br>
for add new shutters to RTS_Link use :<br>
rts_PROG_0 : add a new shutter, return the id of new shutter<br>
rts_ID_MULTIPROG : add a new shutter to the existant ID<br>

you can adding these command in config file and restart hass.It will add a shutter command, press any button for adding a shutter.

example config.yaml :

```
cover:
    platform: rflink
    devices:
        rts_10_0:
            name: Cuisine
        rts_11_0:
            name: Baie Salon
        rts_12_0:
            name: Chambre
        rts_13_0:
            name: Bureau
        rts_14_0:
            name: Salon
        rts_15_0:
            name: Coté nord
        rts_16_0:
            name: Coté sud
```
