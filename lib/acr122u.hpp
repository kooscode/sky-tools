/*
 * MiFare Classic Reader/Writer Wrapper for ACR122U
 * Copyright (C) 2024 Koos du Preez
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * 
 * CREATED BY: Koos du Preez - kdupreez@hotmail.com
 */
            
#ifndef ACR122U_HPP
#define ACR122U_HPP

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <stdint.h>

// using pcsclite for smartcard reading/writing
// see : https://blog.apdu.fr/posts/2010/04/pcsc-sample-in-c

// ** Install pcsc-lite:  sudo apt-get install -y libpcsclite1 libpcsclite-dev libccid opensc
// ** Start pcsd daemon
// ** BLACKLIST drivers in /etc/modprobe.d/blacklist-libnfc.conf 
// **		blacklist nfc
// ** 		blacklist pn533
// ** 		blacklist pn533_usb
//  OR Manuallty unload pn533 driver: sudo modprobe -r pn533_usb
//https://github.com/nfc-tools/libnfc/issues/666

#include <winscard.h>

namespace  xk
{
    #define NFC_UID_SIZE 0x04
    #define NFC_KEY_SIZE 0x06
    #define NFC_BLOCK_SIZE 0x10
    #define NFC_ABITS_SIZE 0x03
    #define NFC_GPBIT_SIZE 0x01
    #define NFC_VENDORINFO_SIZE 0x09
    #define NFC_CARDINFO_SIZE 0x02
    
    #define NFC_BLOCKS_PER_SECTOR 0x04
    #define NFC_SECTORS_PER_CARD 0x10
    #define NFC_CARD_SIZE (NFC_BLOCK_SIZE * NFC_BLOCKS_PER_SECTOR * NFC_SECTORS_PER_CARD)

    #define NFC_OK 0x00
    #define NFC_ERROR_KEYA_SET 0x01
    #define NFC_ERROR_KEYA_AUTH 0x02
    #define NFC_ERROR_BLOCK_READ 0x03
    #define NFC_ERROR_BLOCK_WRITE 0x04    
    #define NFC_ERROR 0x05    

    #define NFC_DEFAULT_KEY_A {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}

    class acr122u
    {
        public:
            struct NFC_Sector
            {
                uint8_t blk0[NFC_BLOCK_SIZE] =	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                uint8_t blk1[NFC_BLOCK_SIZE] =	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                uint8_t blk2[NFC_BLOCK_SIZE] =	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                uint8_t key_A[NFC_KEY_SIZE] = 	{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
                uint8_t abits[NFC_ABITS_SIZE] =	{ 0xFF, 0x07, 0x80}; //DEFAULT ACCESS BITS
                uint8_t gpbit[NFC_GPBIT_SIZE] =	{ 0x69 };
                uint8_t key_B[NFC_KEY_SIZE] =	{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
            };

            struct NFCBlockZero
            {
                uint8_t uid[NFC_UID_SIZE] = {0x00, 0x00, 0x00, 0x00};
                uint8_t bcc = 0x00;
                uint8_t cardinfo[NFC_CARDINFO_SIZE] = {0x08, 0x04};
                uint8_t vendorinfo[NFC_VENDORINFO_SIZE] = {0x00, 0x46, 0x59, 0x25, 0x58, 0x49, 0x10, 0x23, 0x02}; //DEFAULT Vendor info for Skylander NFC's
            };

            // Verify struct sizes match expected NFC card layout
            static_assert(sizeof(NFC_Sector) == 64, "NFC_Sector must be exactly 64 bytes (4 blocks)");
            static_assert(sizeof(NFCBlockZero) == 16, "NFCBlockZero must be exactly 16 bytes (1 block)");

            acr122u();
            virtual ~acr122u();

            std::vector<std::string>    deviceList();
            void                        deviceConnect(const std::string& device_name);
            void                        deviceDisconnect();

            // Helper to find and connect to first ACR122U reader
            // Returns the reader name on success, throws appError on failure
            std::string                 findAndConnect(const std::string& readerPattern = "ACR122U");
            std::string                 getCardInfo(); 

            uint32_t                    sectorAuthKeyA(uint8_t sectorIndex, uint8_t KeyA[NFC_KEY_SIZE]);
            uint32_t                    sectorRead(uint8_t sectorIndex, NFC_Sector& sector_data);
            uint32_t                    sectorWrite(uint8_t sectorIndex, NFC_Sector& sector_data);
            uint32_t                    sectorWrite(uint8_t sectorIndex, NFC_Sector& sector_data, uint8_t block_start, uint8_t block_count);


                                        // Use direct Tag register TX/RX to write to Magic Card Block zero
            bool                        writeBlock0(NFCBlockZero& block_zero);

        private:
            SCARDCONTEXT applicationContext_ = 0x00;
            SCARDHANDLE connectionHandler_ = 0x00;
            DWORD activeProtocol_;

            bool                        sendCommand(uint8_t command[], unsigned short commandLength, uint8_t* outbuffer, uint32_t* outlen); 
            void                        establishContext();
            void                        releaseContext(); 
    };
}

#endif /* ACR122U_HPP */

