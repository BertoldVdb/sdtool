# SD Card Write protection

Most SD cards have a physical write protection switch. This switch is read by the reader instead of being enforced by the card. Of course, many readers ignore this signal and will happily write to a write protected card. Furthermore, Micro SD cards do not have a mechanical switch.

It is relatively unknown that SD/MMC cards also have an electronic write protection system. Every card has two programmable flags, one for temporary write protection and one to lock the card forever. Writing these flags is not supported by most SD host devices. To remedy this, this page presents a program that allows a Linux host to configure the protection register. 

Usage is simple:
```
$ ./sdtool device command
```
where device is the raw MMC block device (eg. /dev/mmcblk0) and command is one of the following:
 * status: Show current write protection setting.
 * unlock: Disable write protection.
 * lock: Enable temporary write protection.
 * permlock: Enable permanent write protection.
 * reset: Send go_idle_state command to all cards on the MMC bus (usually only one). This can sometime restart a stuck card without power cycling the card and host. Make sure it is unmounted first.

Note that you must be able to access the MMC device directly. Some SD card readers connect over USB and export a Mass
Storage Device (ex. /dev/sda), which cannot be used. Preferably, you need an SD card reader that is connected directly to the CPU bus. This is commonly the case on embedded systems and laptops. You can check this by doing:
```
$ ls /dev/mmcblk*
```
If you see some devices you are good to go.

You can find precompiled static binaries (AMD64 and ARM) in the *static* directory. The ARM binary has been verified on a Raspberry Pi B+ and Orange Pi Zero. The AMD64 binary is for standard PCs and has been tested on a Dell Latitude e7270.

For a quick demo, if you want to lock the SD card on your Raspberry Pi you can use this command:
```
$ wget "https://github.com/BertoldVdb/sdtool/blob/master/static/arm-sdtool?raw=true" -O arm-sdtool; chmod +x arm-sdtool; sudo ./arm-sdtool /dev/mmcblk0 lock
```
