# sky-tools

Tools for dumping and cloning Skylander NFC figures using an ACR122U USB NFC reader/writer.

## Features

- **sky-dump**: Read NFC data from a Skylander figure and save to a binary file
- **sky-clone**: Clone Skylander data from a binary file to a blank NFC card
- Automatic UID rewriting for "magic" NFC cards
- Write verification to ensure successful cloning
- Support for all Skylander figure types (1KB MIFARE Classic)

## Requirements

### Hardware
- ACR122U USB NFC Reader/Writer (or compatible)
- Skylander figures to backup
- Blank MIFARE Classic 1K cards (UID-writable "magic" cards recommended)

### Software
- Linux (tested on Ubuntu/Debian)
- g++ with C++17 support
- PC/SC Lite library

## Installation

### Install Dependencies (Debian/Ubuntu)

```bash
sudo apt-get install -y libpcsclite1 libpcsclite-dev libccid opensc
```

### Blacklist Conflicting Drivers

Create `/etc/modprobe.d/blacklist-libnfc.conf`:
```
blacklist nfc
blacklist pn533
blacklist pn533_usb
```

Or manually unload the driver:
```bash
sudo modprobe -r pn533_usb
```

### Start PC/SC Daemon

```bash
sudo systemctl start pcscd
sudo systemctl enable pcscd
```

### Build

```bash
git clone https://github.com/kooscode/sky-tools.git
cd sky-tools
make
```

Binaries will be created in the `bin/` directory.

## Usage

### Dumping a Skylander

1. Connect the ACR122U reader to your computer
2. Place a Skylander figure on the reader
3. Run:

```bash
./bin/sky-dump -f skylander_backup.bin
```

Example output:
```
Looking for ACR122U NFC Reader/Writer...
    Found: ACS ACR122U PICC Interface 01 00
    Connected.
Sector 0 Access Authorized
Reading Sector 0:
    Done. [UID=9F8909AF]
Reading ALL Sectors:
    ...............    Done.
Dumping to file skylander_backup.bin:
    Done.
```

### Cloning to a Blank Card

1. Connect the ACR122U reader to your computer
2. Place a **blank** MIFARE Classic 1K card on the reader
3. Run:

```bash
./bin/sky-clone -f skylander_backup.bin
```

4. If the card UID doesn't match, you'll be prompted to remove and replace the card after UID rewrite

Example output:
```
STEP 1: Reading Skylander file "skylander_backup.bin"
    Skylander File UID : [UID=9F8909AF]
STEP 2: Connecting to ACR122U NFC Reader
    Found: ACS ACR122U PICC Interface 01 00
    Connected.
STEP 3: Authenticating Card and Ensure Skylander UID Match
    Authorized (Default KeyA)
    Reading Sector 0: Done [UID=9F8909AF]
    UID's match - proceeding with clone
STEP 4: Cloning Skylander Sectors:
[dARW0_4][dARW0_4]...[dARW0_4]
STEP 5: Verifying Written Data:
[ARV][ARV]...[ARV]
Clone Complete - Verification PASSED!
```

### Output Legend

During cloning:
- `d` = Used default key (blank card)
- `A` = Authentication successful
- `R` = Read successful
- `W0_4` = Wrote 4 blocks starting at block 0
- `x` = Operation failed

During verification:
- `A` = Authentication successful
- `R` = Read successful
- `V` = Verification passed

## Command Line Options

### sky-dump
```
Usage: sky-dump [OPTIONS]

Options:
  -f <filename>    Output BIN filename (required)
  -h, --help       Show help
```

### sky-clone
```
Usage: sky-clone [OPTIONS]

Options:
  -f <filename>    Source BIN filename (required)
  -h, --help       Show help
```

## How It Works

### Skylander NFC Structure
- Skylanders use MIFARE Classic 1K NFC chips (1024 bytes)
- 16 sectors, 4 blocks per sector (64 bytes per sector)
- Sector 0 contains the UID and Skylander identification
- Each sector is protected with computed keys based on the UID

### Key Generation
- Sector 0 uses a fixed Skylander key
- Sectors 1-15 use keys computed via CRC48 algorithm using the card UID
- This ensures each Skylander has unique sector keys

### Cloning Process
1. Read source Skylander data including UID
2. If target card UID doesn't match, rewrite block 0 (requires magic card)
3. Authenticate and write all sectors with appropriate keys
4. Verify all written data matches source

## Troubleshooting

### "No NFC readers found"
- Ensure ACR122U is connected via USB
- Check `lsusb` for the device
- Ensure pcscd daemon is running: `sudo systemctl status pcscd`

### "No smart card inserted"
- Place a Skylander or NFC card on the reader
- Ensure card is properly positioned on the reader

### "NFC Sector 0 Authentication Failed"
- Card may not be a valid Skylander or blank MIFARE Classic
- Try a different card

### "Can NOT overwrite a different Skylander"
- You cannot clone to a card that already contains a different Skylander
- Use a blank card instead

### UID Rewrite Fails
- Ensure you're using a UID-writable "magic" card
- Regular MIFARE Classic cards have read-only UIDs

## Supported Cards

- **For cloning**: MIFARE Classic 1K with writable UID ("magic" cards, Gen1a/Gen2)
- **Original Skylanders**: Standard MIFARE Classic 1K

## License

GNU Lesser General Public License v2.1 - see [LICENSE](LICENSE)

## Credits

- Original code by Koos du Preez (kdupreez@hotmail.com)
- Skylander key algorithm from [nfc.toys](https://nfc.toys/interop-sky.html)
