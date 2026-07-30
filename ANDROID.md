# Guide
## Using sdtool on Android (Root Required)

Because modern laptops don't have built-in card readers, we will use Android phone's built in sd card slots and termux as linux. `sdtool` can be compiled and run on a rooted Android device using Termux. This guide provides step-by-step instructions for unlocking SD cards with temporary write protection.

### Prerequisites

- A rooted Android phone (Magisk or SuperSU) 
- The Termux app installed

### Important Notes

- **Built-in card reader required**: Because USB card readers won't work, we utilized android phone's sd-card slot as built in and termux as linux. The SD card must appear as `/dev/block/mmcblk*` (not `/dev/sd*`).
- **Root access required**: All commands must be run with `sudo`.
- **Data loss warning**: Unlocking and formatting will permanently erase all data on the card.
- **TESTED TO WORK with temporary write protection only for now**: Check status first to confirm.

---

### Step-by-Step Guide

#### 1. Install Required Packages

Open Termux and update the package list:

```bash
sudo apt update && sudo apt upgrade

```

Install the necessary tools:

```bash
sudo apt install git make clang dosfstools parted
```

If `sudo` is not installed, install it first:

```bash
pkg install sudo
```

#### 2. Clone and Compile `sdtool`

```bash
git clone https://github.com/BertoldVdb/sdtool
cd sdtool
make
```

#### 3. Identify Your SD Card

List all block devices to find your SD card:

```bash
sudo lsblk
```

**Important:** Look for a device named `mmcblk0` or `mmcblk1` with a size matching your SD card. (It may be found 5-10 lines from bottom).

Example output showing a correctly detected card:
```
mmcblk1      179:32   0 238.3G  0 disk
└─mmcblk1p1  179:33   0 238.3G  0 part /mnt/pass_through/0/C6B4-1C0C
```

#### 4. Check the Current Write Protection Status

```bash
sudo ./sdtool /dev/block/mmcblk1 status
```

Replace `/dev/block/mmcblk1` with your actual device path if different.

Expected output for a locked BYJU'S card:
```
[+] Found RCA for /dev/block/mmcblk1: AAAA.
[+] Card CSD: 400E002B5B590007725F7F800A405015.
[+] Write protection state: Temporary.
```

> **Note:** If the state shows "Permanent" instead of "Temporary", this method will not work.

#### 5. Unmount the SD Card

Before unlocking, unmount the card to prevent conflicts:

```bash
sudo umount /dev/block/mmcblk1p1
```

Or unmount by mount point (replace with your actual mount point):

```bash
sudo umount /mnt/pass_through/0/C6B4-1C0C
```

#### 6. Unlock the SD Card

Run the unlock command:

```bash
sudo ./sdtool /dev/block/mmcblk1 unlock
```

Successful output:
```
[+] Found RCA for /dev/block/mmcblk1: AAAA.
[+] Writing CSD.
[+] Write protection state: Off.
```

#### 7. Verify the Unlock

Confirm the write protection is now disabled:

```bash
sudo ./sdtool /dev/block/mmcblk1 status
```

Expected output:
```
[+] Found RCA for /dev/block/mmcblk1: AAAA.
[+] Card CSD: 400E002B5B590007725F7F800A404027.
[+] Write protection state: Off.
```

---

### Formatting the Unlocked Card

Now that the write protection is disabled, you have two options:

#### Option A: Format via Android Settings (Recommended - Easiest)

1. Go to **Settings > Storage**
2. Find your SD card
3. Tap on **"Format"** or **"Format as portable storage"**
4. Confirm the format

The card will now be fully writable and recognized as normal storage.

#### Option B: Format via Termux (Manual)

If you prefer to format directly from the command line:

1. Delete the existing partition:
```bash
sudo parted /dev/block/mmcblk1 rm 1
```

2. Create a new partition table:
```bash
sudo parted /dev/block/mmcblk1 mklabel msdos
```

3. Create a new primary partition:
```bash
sudo parted /dev/block/mmcblk1 mkpart primary fat32 0% 100%
```

4. Set the boot flag (optional):
```bash
sudo parted /dev/block/mmcblk1 set 1 boot on
```

5. Format the partition as FAT32:
```bash
sudo mkfs.vfat /dev/block/mmcblk1p1
```

6. Verify the new partition:
```bash
sudo parted /dev/block/mmcblk1 print
```

Expected output:
```
Model: SD SC256 (sd/mmc)
Disk /dev/block/mmcblk1: 256GB
Partition Table: msdos
Number  Start   End    Size   Type     File system  Flags
 1      1049kB  256GB  256GB  primary  fat32        boot, lba
```

---

### Full Command Summary

Here is the complete workflow in one block:

```bash
# Install dependencies
sudo apt update && sudo apt upgrade
sudo apt install git make clang dosfstools parted

# Clone and compile sdtool
git clone https://github.com/BertoldVdb/sdtool
cd sdtool
make

# Check status
sudo ./sdtool /dev/block/mmcblk1 status

# Unmount the card
sudo umount /dev/block/mmcblk1p1

# Unlock the card
sudo ./sdtool /dev/block/mmcblk1 unlock

# Verify unlock
sudo ./sdtool /dev/block/mmcblk1 status

# Format via Android Settings or use commands below:
sudo parted /dev/block/mmcblk1 rm 1
sudo parted /dev/block/mmcblk1 mklabel msdos
sudo parted /dev/block/mmcblk1 mkpart primary fat32 0% 100%
sudo parted /dev/block/mmcblk1 set 1 boot on
sudo mkfs.vfat /dev/block/mmcblk1p1

# Reboot your phone to finalize
sudo reboot
```

---

### Troubleshooting

| Issue | Solution |
|-------|----------|
| `sdtool` shows card as `sdX` | You're using a USB card reader. Use a built-in reader that connects to the CPU bus. |
| `Device or resource busy` | Reboot your phone and try again immediately after boot. |
| `Write protection state: Permanent` | This method only works for "Temporary" locks. The card has a permanent hardware lock. |
| `sudo: command not found` | Install sudo with `pkg install sudo` |
| `make: gcc: No such file or directory` | Install clang: `sudo apt install clang` |

---

### Tested Configurations

This method has been successfully tested on:

| Device | SD Card | Result |
|--------|---------|--------|
| Rooted Android Phone (Termux) | 256GB | ✅ Unlocked |
| Rooted Android Phone (Termux) | 29.7GB | ✅ Unlocked |

---

### Credits

- [sdtool](https://github.com/BertoldVdb/sdtool) by BertoldVdb
- Community contributors for the Android implementation guide

---

### Disclaimer

This guide is for educational purposes only. Unlocking and formatting will permanently erase all data on the SD card. Use at your own risk. The authors are not responsible for any data loss or hardware damage.

