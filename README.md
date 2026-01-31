# sky-tools

Tools for dumping, cloning, and resetting Skylander NFC figures using an ACR122U USB NFC reader/writer.

## Features

- **sky-dump**: Read NFC data from a Skylander figure and save to a binary file
- **sky-clone**: Clone Skylander data from a binary file to a blank NFC card
- **sky-reset**: Factory reset a Skylander (zero XP, money, skills, hats, hero points)
- **Skylander Identification**: Automatically identifies 180+ characters from all games
- Automatic UID rewriting for "magic" NFC cards
- Write verification to ensure successful cloning
- Support for all Skylander figure types (1KB MIFARE Classic)

## Supported Games

All Skylanders from every game are supported:
- Spyro's Adventure (2011)
- Giants (2012)
- SWAP Force (2013)
- Trap Team (2014)
- SuperChargers (2015)
- Imaginators (2016) - including Senseis and Creation Crystals

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
# Auto-generate filename from Skylander info
./bin/sky-dump

# Or specify a custom filename
./bin/sky-dump -f skylander_backup.bin
```

When no filename is provided, it auto-generates one using the format:
- Regular Skylanders: `name_game_element_uid.bin`
- Creation Crystals/Traps: `type_game_element_uid.bin` (avoids redundancy)

Example output:
```
Looking for ACR122U NFC Reader/Writer...
    Found: ACS ACR122U PICC Interface 01 00
    Connected.
Sector 0 Access Authorized
Reading Sector 0:
    Done. [UID=CFD6E5FA]
Skylander: Jawbreaker (Trap Master, Tech)
Reading ALL Sectors:
    ...............    Done.
Dumping to file jawbreaker_trap_team_tech_cfd6e5fa.bin:
    Done.
```

Example filenames:
- `jawbreaker_trap_team_tech_cfd6e5fa.bin` (Trap Master from Trap Team)
- `spyro_spyros_adventure_magic_12345678.bin` (Original Spyro)
- `creation_crystal_imaginators_fire_9f8909af.bin` (Fire Creation Crystal)

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
    Skylander File UID : [UID=CFD6E5FA]
    Skylander: Jawbreaker (Trap Master, Tech)
STEP 2: Connecting to ACR122U NFC Reader
    Found: ACS ACR122U PICC Interface 01 00
    Connected.
STEP 3: Authenticating Card and Ensure Skylander UID Match
    Authorized (Default KeyA)
    Reading Sector 0: Done [UID=CFD6E5FA]
    UID's match - proceeding with clone
STEP 4: Cloning Skylander Sectors:
[dARW0_4][dARW0_4]...[dARW0_4]
STEP 5: Verifying Written Data:
[ARV][ARV]...[ARV]
Clone Complete - Verification PASSED!
```

### Resetting a Skylander

Reset a Skylander to factory blank state (removes all progress):

```bash
# Reset using NFC reader (place Skylander on reader)
./bin/sky-reset

# Reset from a dump file
./bin/sky-reset -f skylander_backup.bin -o skylander_reset.bin

# Dry run (show what would happen without making changes)
./bin/sky-reset -f skylander_backup.bin -n
```

Example output:
```
STEP 1: Reading Skylander from file "skylander_backup.bin"
    Skylander UID: CFD6E5FA
    Skylander: Jawbreaker (Trap Master, Tech)
STEP 2: Decrypting Skylander data
    Decryption complete.
    Active Area: 0
    Current XP: 6707094
    Current Money: 49160
    Sequence counters: Area0=158, Area1=29
STEP 3: Resetting Skylander data
    Cleared XP, Money, Skills, Hats, Hero data
    New sequence counter: 159
STEP 4: Recalculating checksums
    Checksums updated.
STEP 5: Re-encrypting Skylander data
    Encryption complete.
STEP 6: Writing reset Skylander to file "skylander_reset.bin"
    Done.

Reset Complete!
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
  -f <filename>    Output BIN filename (optional - auto-generates if not provided)
  -h, --help       Show help
```

### sky-clone
```
Usage: sky-clone [OPTIONS]

Options:
  -f <filename>    Source BIN filename (required)
  -h, --help       Show help
```

### sky-reset
```
Usage: sky-reset [OPTIONS]

Options:
  -f, --file <filename>    Input BIN file (if not using NFC reader)
  -o, --output <filename>  Output BIN file (default: overwrite input or write to card)
  -n, --dry-run            Don't write changes, just show what would happen
  -h, --help               Show help
```

## How It Works

### Skylander NFC Structure
- Skylanders use MIFARE Classic 1K NFC chips (1024 bytes)
- 16 sectors, 4 blocks per sector (64 bytes per sector)
- Sector 0 contains the UID and Skylander identification
- Character ID stored at offset 0x10 (used for identification)
- Each sector is protected with computed keys based on the UID

### Key Generation
- Sector 0 uses a fixed Skylander key
- Sectors 1-15 use keys computed via CRC48 algorithm using the card UID
- This ensures each Skylander has unique sector keys

### Data Encryption
- Blocks 8+ are encrypted using AES-128-ECB
- Key derived from: MD5(sector0[0:32] + blockIndex + Activision copyright string)
- Two data areas (blocks 0x08-0x0F and 0x24-0x2F) for redundancy
- Sequence counter determines which area is active

### Checksums
- CRC-16/CCITT-FALSE checksums protect data integrity
- Type 0: Sector 0 validation
- Type 1: Data area header
- Type 2: First data blocks
- Type 3: Remaining data blocks

### Cloning Process
1. Read source Skylander data including UID
2. If target card UID doesn't match, rewrite block 0 (requires magic card)
3. Authenticate and write all sectors with appropriate keys
4. Verify all written data matches source

### Reset Process
1. Read and decrypt Skylander data
2. Zero out XP, money, skills, hats, hero points, heroic challenges
3. Increment sequence counter
4. Recalculate all checksums
5. Re-encrypt and write back

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
- Character ID database from [skylandersNFC](https://gist.github.com/skylandersNFC/4f0348c7e66fe9ab28e2ac3b82e549e2)
