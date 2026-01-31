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

#include "acr122u.hpp"
#include "appError.hpp"

#include <chrono>
#include <thread>

using namespace std::this_thread;     // sleep_for, sleep_until
using namespace std::chrono_literals; // ns, us, ms, s, h, etc.
using std::chrono::system_clock;

namespace xk
{
    acr122u::acr122u()
    {
        establishContext();
    }

    acr122u::~acr122u()
    {
        if (applicationContext_ != 0x00)
            releaseContext();

        if (connectionHandler_ != 0x00)
            deviceDisconnect();
    }

    void acr122u::establishContext() 
    {
        LONG status = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &applicationContext_);
        if (status != SCARD_S_SUCCESS)
            throw new appError("acr122u::establishContext", std::string(pcsc_stringify_error(status)), status);
    }

    void acr122u::releaseContext() 
    {
        LONG status = SCardReleaseContext(applicationContext_);
        if (status != SCARD_S_SUCCESS)
            throw new appError("acr122u::releaseContext", std::string(pcsc_stringify_error(status)), status);
    }

    std::vector<std::string> acr122u::deviceList() 
    {
        std::vector<std::string> reader_list;

        LPSTR reader_names = NULL;
        DWORD alloc_options = SCARD_AUTOALLOCATE;

        LONG status = SCardListReaders(applicationContext_, NULL, (LPSTR)&reader_names, &alloc_options);
        
        if (status == SCARD_S_SUCCESS) 
        {
            char *reader_name = reader_names;
            while (*reader_name) 
            {
                reader_list.push_back(std::string(reader_name));
                reader_name += strlen(reader_name) +1;
            }
        } 
        else 
        {
            throw new appError("acr122u::deviceList", std::string(pcsc_stringify_error(status)), status);
        }
        
        // free allocated memory for list of readers
        status = SCardFreeMemory(applicationContext_, reader_names);
        if (status != SCARD_S_SUCCESS)
            throw new appError("acr122u::deviceList (SCardFreeMemory)", std::string(pcsc_stringify_error(status)), status);

        return reader_list;
    }

    void acr122u::deviceConnect(const std::string& device_name) 
    {
        activeProtocol_ = -1;
        LONG status = SCardConnect(applicationContext_, device_name.c_str(), SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &connectionHandler_, &activeProtocol_);
        if (status != SCARD_S_SUCCESS)
            throw new appError("acr122u::deviceConnect", std::string(pcsc_stringify_error(status)), status);
    }

    void acr122u::deviceDisconnect()
    {
        if (connectionHandler_ != 0x00)
        {
            LONG status = SCardDisconnect(connectionHandler_, SCARD_RESET_CARD);
            if (status != SCARD_S_SUCCESS)
                throw new appError("acr122u::deviceDisconnect", std::string(pcsc_stringify_error(status)), status);

            connectionHandler_ = 0x00;
        }
    }

    std::string acr122u::findAndConnect(const std::string& readerPattern)
    {
        std::vector<std::string> readers = deviceList();

        if (readers.empty())
            throw new appError("acr122u::findAndConnect", "No NFC readers found");

        // Find first reader matching the pattern
        std::string readerName;
        for (const auto& reader : readers)
        {
            if (reader.find(readerPattern) != std::string::npos)
            {
                readerName = reader;
                break;
            }
        }

        if (readerName.empty())
            throw new appError("acr122u::findAndConnect", "No " + readerPattern + " compatible reader found");

        deviceConnect(readerName);
        return readerName;
    }

    std::string acr122u::getCardInfo() 
    {
        std::string card_info = "";

        BYTE ATR[MAX_ATR_SIZE] = "";
        DWORD ATRLength = sizeof(ATR);
        char readerName[MAX_READERNAME] = "";
        DWORD readerLength = sizeof(readerName);
        DWORD readerState;
        DWORD readerProtocol;
        
        LONG status = SCardStatus(connectionHandler_, readerName, &readerLength, &readerState, &readerProtocol, ATR, &ATRLength);
        if (status == SCARD_S_SUCCESS) 
        {
            card_info =  std::string(readerName);
        } 
        else 
        {
            throw new appError("acr122u::getCardInfo", std::string(pcsc_stringify_error(status)), status);
        }

        return card_info;
    }

    bool acr122u::sendCommand(uint8_t command[], unsigned short commandLength, uint8_t* outbuffer, uint32_t* outlen) 
    {
        const SCARD_IO_REQUEST *pioSendPci;
        SCARD_IO_REQUEST pioRecvPci;

        switch(activeProtocol_) 
        {
            case SCARD_PROTOCOL_T0:
                pioSendPci = SCARD_PCI_T0;
                break;
            case SCARD_PROTOCOL_T1:
                pioSendPci = SCARD_PCI_T1;
                break;
            default:
                throw new appError("acr122u::sendCommand", "Invalid  Protocol");
        }

        DWORD dw_outlen = *outlen;
        LONG status = SCardTransmit(connectionHandler_, pioSendPci, command, commandLength, &pioRecvPci, outbuffer, &dw_outlen);
        *outlen = dw_outlen;

        if (status != SCARD_S_SUCCESS) 
            throw new appError("acr122u::sendCommand", std::string(pcsc_stringify_error(status)), status);

        return true;
    }

    uint32_t acr122u::sectorAuthKeyA(uint8_t sectorIndex, uint8_t KeyA[NFC_KEY_SIZE])
    {
        uint8_t outbuff[1024];
        uint32_t outlen = sizeof(outbuff);
        memset(outbuff, 0x00, outlen);
        bool cmd_ok = false;

        uint8_t blockNumber = sectorIndex * NFC_BLOCKS_PER_SECTOR;

        // Set Key A
        uint8_t authenticationKeysCommand[11] = { 0xFF, 0x82, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
        unsigned short authenticationKeysCommandLength = sizeof(authenticationKeysCommand);
        memcpy(authenticationKeysCommand + 0x05, KeyA, NFC_KEY_SIZE);
        cmd_ok = sendCommand(authenticationKeysCommand, authenticationKeysCommandLength, outbuff,  &outlen);
        if (!cmd_ok || outbuff[outlen-2] != 0x90)
            return NFC_ERROR_KEYA_SET;

        // Reset outbuff
        outlen = sizeof(outbuff);
        memset(outbuff, 0x00, outlen);

        // Authenticate
        uint8_t authenticateCommand[] = { 0xFF, 0x86, 0x00, 0x00, 0x05, 0x01, 0x00, blockNumber, 0x60, 0x00 };
        unsigned short authenticateCommandLength = sizeof(authenticateCommand);
        cmd_ok = sendCommand(authenticateCommand, authenticateCommandLength, outbuff,  &outlen);
        if (!cmd_ok || outbuff[outlen-2] != 0x90)
            return NFC_ERROR_KEYA_AUTH;	

        return NFC_OK;
    }

    uint32_t acr122u::sectorRead(uint8_t sectorIndex, acr122u::NFC_Sector& sector_data)
    {
        uint8_t outbuff[1024];
        uint32_t outlen = sizeof(outbuff);
        memset(outbuff, 0x00, outlen);
        bool cmd_ok = false;

        uint8_t blockNumber = sectorIndex * NFC_BLOCKS_PER_SECTOR;

        //make backup copy of KeyA
        uint8_t keyA[NFC_KEY_SIZE];
        memcpy(keyA, sector_data.key_A, NFC_KEY_SIZE);

        //Read Blocks for entire sector
        uint8_t* psector = static_cast<uint8_t*>(static_cast<void*>(&sector_data));
        for (uint8_t blk_idx = 0; blk_idx < NFC_BLOCKS_PER_SECTOR; blk_idx++)
        {
            uint8_t readCommand[] = { 0xFF, 0xB0, 0x00, static_cast<uint8_t>(blockNumber + blk_idx), 0x10 };
            unsigned short readCommandLength = sizeof(readCommand);
            cmd_ok = sendCommand(readCommand, readCommandLength, outbuff,  &outlen);
            if (!cmd_ok || outbuff[outlen-2] != 0x90)
                return NFC_ERROR_BLOCK_READ;

            memcpy(psector + NFC_BLOCK_SIZE * blk_idx, outbuff, NFC_BLOCK_SIZE);
        }

        //Reading from Last Block will CLEAR KeyA to all Zeros, Restore KeyA from backup
        memcpy(sector_data.key_A, keyA, NFC_KEY_SIZE);

        return NFC_OK;
    }

    uint32_t acr122u::sectorWrite(uint8_t sectorIndex, acr122u::NFC_Sector& sector_data)
    {
        return acr122u::sectorWrite(sectorIndex, sector_data, 0, NFC_BLOCKS_PER_SECTOR);
    }


    uint32_t acr122u::sectorWrite(uint8_t sectorIndex, NFC_Sector& sector_data, uint8_t block_start, uint8_t block_count)
    {
        uint8_t outbuff[1024];
        uint32_t outlen = sizeof(outbuff);
        memset(outbuff, 0x00, outlen);
        bool cmd_ok = false;

        uint8_t blockNumber = sectorIndex * NFC_BLOCKS_PER_SECTOR;

        //Write Blocks for entire sector
        uint8_t* psector = static_cast<uint8_t*>(static_cast<void*>(&sector_data));
        for (uint8_t blk_idx = block_start; blk_idx < block_start + block_count; blk_idx++)
        {
            uint8_t writeCommand[21] = { 0xFF, 0xD6, 0x00, static_cast<uint8_t>(blockNumber + blk_idx), 0x10, 
                                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // <-- BLOCK DATA 
            
            //copy data to write to command
            memcpy(writeCommand + 5, psector + NFC_BLOCK_SIZE * blk_idx, NFC_BLOCK_SIZE);
            unsigned short writeCommandLength = sizeof(writeCommand);

            // send it. 
            // If writing Sector 0, DO NOT attempt write on Block Zero
            if (blockNumber + blk_idx > 0)
            {
                cmd_ok = sendCommand(writeCommand, writeCommandLength, outbuff,  &outlen);
                if (!cmd_ok || outbuff[outlen-2] != 0x90)
                    return NFC_ERROR_BLOCK_WRITE;
            }


        }

        return NFC_OK;
    }

    // Use direct Tag register TX/RX to write to Magic Card Block zero
    bool acr122u::writeBlock0(acr122u::NFCBlockZero& block_zero)
    {
        uint8_t outbuff[1024];
        uint32_t outlen = sizeof(outbuff);
        memset(outbuff, 0x00, outlen);

        // uint8_t wc1[13] = { 0xFF, 0x00, 0x00, 0x00, 0x08, [              ** 8 Byte CMD **                ] };
        uint8_t wc1[13] = { 0xFF, 0x00, 0x00, 0x00, 0x08,     0xD4, 0x08, 0x63, 0x02, 0x00, 0x63, 0x03, 0x00  };
        if (!sendCommand(wc1, sizeof(wc1), outbuff,  &outlen))
            return false;

        sleep_for(250ms);

        // Reset outbuff
        outlen = sizeof(outbuff);
        memset(outbuff, 0x00, outlen);
        // uint8_t wc1[13] = { 0xFF, 0x00, 0x00, 0x00, 0x06, [          ** 6 Byte CMD **         ] };
        uint8_t wc2[11] = { 0xFF, 0x00, 0x00, 0x00, 0x06,      0xD4, 0x42, 0x50, 0x00, 0x57, 0xCD  };
        if (!sendCommand(wc2, sizeof(wc2), outbuff,  &outlen))
            return false;

        sleep_for(250ms);

        // Reset outbuff
        outlen = sizeof(outbuff);
        memset(outbuff, 0x00, outlen);
        // uint8_t wc3[10] = { 0xFF, 0x00, 0x00, 0x00, 0x05, [      ** 5 Byte CMD **      ] };
        uint8_t wc3[10] = { 0xFF, 0x00, 0x00, 0x00, 0x05,     0xD4, 0x08, 0x63, 0x3D, 0x07  };
        if (!sendCommand(wc3, sizeof(wc3), outbuff,  &outlen))
            return false;

        sleep_for(250ms);

        // Reset outbuff
        outlen = sizeof(outbuff);
        memset(outbuff, 0x00, outlen);
        // uint8_t wc4[8] = { 0xFF, 0x00, 0x00, 0x00, 0x03, [** 3 Byte CMD **] };
        uint8_t wc4[8] = { 0xFF, 0x00, 0x00, 0x00, 0x03,     0xD4, 0x42, 0x40  };
        if (!sendCommand(wc4, sizeof(wc4), outbuff,  &outlen))
            return false;

        sleep_for(250ms);

        // Reset outbuff
        outlen = sizeof(outbuff);
        memset(outbuff, 0x00, outlen);
        // uint8_t wc5[10] = { 0xFF, 0x00, 0x00, 0x00, 0x05, [      ** 5 Byte CMD **      ] };
        uint8_t wc5[10] = { 0xFF, 0x00, 0x00, 0x00, 0x05,     0xD4, 0x08, 0x63, 0x3D, 0x00  };
        if (!sendCommand(wc5, sizeof(wc5), outbuff,  &outlen))
            return false;

        sleep_for(250ms);

        // Reset outbuff
        outlen = sizeof(outbuff);
        memset(outbuff, 0x00, outlen);
        //uint8_t wc6[8] = { 0xFF, 0x00, 0x00, 0x00, 0x03, [** 3 Byte CMD **] };
        uint8_t wc6[8] = { 0xFF, 0x00, 0x00, 0x00, 0x03,    0xD4, 0x42, 0x43};
        if (!sendCommand(wc6, sizeof(wc6), outbuff,  &outlen))
            return false;

        sleep_for(250ms);

        // Reset outbuff
        outlen = sizeof(outbuff);
        memset(outbuff, 0x00, outlen);
        //uint8_t wc7[13] = { 0xFF, 0x00, 0x00, 0x00, 0x08, [              ** 8 Byte CMD **                ] };
        uint8_t wc7[13] = { 0xFF, 0x00, 0x00, 0x00, 0x08,    0xD4, 0x08, 0x63, 0x02, 0x80, 0x63, 0x03, 0x80  };
        if (!sendCommand(wc7, sizeof(wc7), outbuff,  &outlen))
            return false;

        sleep_for(250ms);

        // Reset outbuff
        outlen = sizeof(outbuff);
        memset(outbuff, 0x00, outlen);

        //setup cmd bytes
        // uint8_t wc8[26] = { 0xFF, 0x00, 0x00, 0x00, 0x15, [ ** 21 Byte CMD which includes Block 0 **  ] };
        uint8_t wc8[26] = { 0xFF, 0x00, 0x00, 0x00, 0x15,   0xD4, 0x40, 0x01, 0xA0, 0x00, 
                                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // <-- Block 0
        
        
        //compute BCC
        block_zero.bcc = block_zero.uid[0] ^ block_zero.uid[1] ^ block_zero.uid[2] ^ block_zero.uid[3];        

        //copy block zero into command
        uint8_t* pbz = static_cast<uint8_t*>(static_cast<void*>(&block_zero));
        memcpy(wc8+0x0A, pbz, sizeof(acr122u::NFCBlockZero));
        
        //send it
        if (!sendCommand(wc8, sizeof(wc8), outbuff,  &outlen))
            return false;

        sleep_for(250ms);

        return true;
    }

}