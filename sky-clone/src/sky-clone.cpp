#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fstream>
#include <string>
#include <iostream>
#include <chrono>
#include <thread>

#include "cxxopts.hpp"
#include "appError.hpp"
#include "acr122u.hpp"
#include "skylanderNFC.hpp"
#include "skylanderDB.hpp"

using namespace std::this_thread;     // sleep_for, sleep_until
using namespace std::chrono_literals; // ns, us, ms, s, h, etc.
using std::chrono::system_clock;

enum sky_auth
{
	SKY_AUTH_NONE = 0,
	SKY_AUTH_SKYKEY = 1,
	SKY_AUTH_DEFAULT = 2
};

sky_auth authSector0(xk::acr122u& nfc)
{
	sky_auth retval = SKY_AUTH_NONE;
	uint8_t nfc_key_a[] = NFC_DEFAULT_KEY_A;
	uint8_t sky_key_a[] = SKYLANDER_BLOCK_0_KEY_A;

	if (nfc.sectorAuthKeyA(0, nfc_key_a) == NFC_OK)
		retval = SKY_AUTH_DEFAULT;
	else if (nfc.sectorAuthKeyA(0, sky_key_a) == NFC_OK)
		retval = SKY_AUTH_SKYKEY;
	else
		retval = SKY_AUTH_NONE;

	return retval;
}

bool writeSector0Block0(xk::acr122u& nfc, const std::string& reader_name, xk::acr122u::NFC_Sector& skylander_sector0)
{
	bool retval = false;

	//create a new NFCBlockZero using the Skylander block 0 data
	xk::acr122u::NFCBlockZero nfc_block0;
	memcpy(&nfc_block0, skylander_sector0.blk0, sizeof(xk::acr122u::NFCBlockZero));

	//write new UID to card
	std::cout << "\tRewriting NFC UID:" << std::endl;
	std::cout << "\t\t[UID=" << xk::skylanderNFC::toHexStr(nfc_block0.uid, NFC_UID_SIZE) << "]" << std::endl;
	std::cout << "\t\t[BCC=" << std::hex << std::uppercase << (int) nfc_block0.bcc << "]" << std::nouppercase << std::endl;
	std::cout << "\t\t[CARD_INFO=" << xk::skylanderNFC::toHexStr(nfc_block0.cardinfo, NFC_CARDINFO_SIZE) << "]" << std::endl;
	std::cout << "\t\t[VENDOR_INFO=" << xk::skylanderNFC::toHexStr(nfc_block0.vendorinfo, NFC_VENDORINFO_SIZE) << "]" << std::endl;

	std::cout << "\tDisconnect NFC Reader/Writer: ";
	nfc.deviceDisconnect();
	std::cout << "Disconnected" << std::endl; 
	sleep_for(2000ms);

	// connect to device
	std::cout << "\tReconnecting NFC Reader/Writer: ";
	nfc.deviceConnect(reader_name);
	std::cout << "Connected" << std::endl; 

	//set Write Block Zero
	std::cout << "\tWriting New Block 0 (please wait..): " << std::flush;
	retval = nfc.writeBlock0(nfc_block0);

	if (!retval)
	{
		std::cout << "Failed" << std::endl;
	}
	else
	{
		std::cout << "Done" << std::endl; 

		std::cout << "\tDisconnect NFC Reader/Writer: ";
		nfc.deviceDisconnect();
		std::cout << "Disconnected" << std::endl; 

		std::string in_str;
		std::cout << std::endl << "** Please remove/replace card - press ENTER to continue **" << std::endl;
		std::getline(std::cin, in_str);

		// connect to device
		std::cout << "\tReconnecting NFC Reader/Writer: ";
		nfc.deviceConnect(reader_name);
		std::cout << "Connected" << std::endl; 	
	}

	return retval;
}

int main(int argc, char** argv) 
{   
 	std::string sky_file_name = "";

	//struct to keep file and card sectors
	xk::acr122u::NFC_Sector sky_file[NFC_SECTORS_PER_CARD];
	xk::acr122u::NFC_Sector sky_card[NFC_SECTORS_PER_CARD];

	//setup cmdline options
	cxxopts::Options app_options("sky-clone", "Skylander NFC Clone Tool");
	app_options.add_options()
  		("f", "Source BIN filename", cxxopts::value<std::string>())
		("h,help", "Show Syntax");

	// parse supplied options
	cxxopts::ParseResult app_params;
	try 
	{
		app_params = app_options.parse(argc, argv);
	} 
	catch (const cxxopts::exceptions::exception& e)
	{
    	std::cout << app_options.help() << std::endl;
		exit(-1);
	}

    // print help if required..
	if ((app_params.count("h")) || (app_params.count("f") > 0))
	{
		sky_file_name =  app_params["f"].as<std::string>();
	}
	else
	{
    	std::cout << app_options.help() << std::endl;
		exit(-1);
	}
	
	std::cout << "STEP 1:\tReading Skylander file \"" << sky_file_name << "\"" <<std::endl;

	// Open file
	std::ifstream skyfile(sky_file_name, std::ios_base::binary);

	if (!skyfile.is_open()) 
	{
		std::cout << "\n** ERROR - Could not open Skylander BIN file: " << sky_file_name << std::endl;
		return -1;
	}

	// Get length of skyfile
	skyfile.seekg(0, std::ios::end);
	uint32_t file_size = skyfile.tellg();
	skyfile.seekg(0, std::ios::beg);

	// Check file size
	if (file_size != NFC_CARD_SIZE) 
	{
		std::cout << "\n** ERROR - Skylander BIN File size mismatch : " << file_size << " vs " << NFC_CARD_SIZE << std::endl;
		return -1;
	}

	// Read skylander from file
	skyfile.read(static_cast<char*>(static_cast<void*>(&sky_file)), NFC_CARD_SIZE);
	skyfile.close();

	//Extract Skylander File UID
	uint8_t file_uid[4];
	mempcpy(file_uid, sky_file[0].blk0, NFC_UID_SIZE);
	std::cout << "\tSkylander File UID : [UID=" << xk::skylanderNFC::toHexStr(file_uid,  NFC_UID_SIZE) << "]" << std::endl;

	// Identify Skylander from file
	uint8_t* file_data = reinterpret_cast<uint8_t*>(sky_file);
	uint16_t charId = xk::skylanderDB::getCharacterId(file_data);
	const xk::SkylanderInfo* info = xk::skylanderDB::getInfo(charId);
	if (info)
	{
		std::cout << "\tSkylander: " << info->name << " (" << xk::skylanderDB::getTypeString(info->type) << ", " << info->element << ")" << std::endl;
	}
	else
	{
		std::cout << "\tSkylander: Unknown (ID: 0x" << std::hex << charId << std::dec << ")" << std::endl;
	}

	std::cout << "STEP 2:\tConnecting to ACR122U NFC Reader" << std::endl;
	try
	{
		xk::acr122u nfc;
		std::string reader_name = nfc.findAndConnect();
		std::cout << "\tFound: " << reader_name << std::endl;
		std::cout << "\tConnected." << std::endl;

		// loop and attempt to rewrite UID if does not match
		std::cout << "STEP 3:\tAuthenticating Card and Ensure Skylander UID Match" << std::endl;
		uint32_t retries_max = 3;
		uint32_t retries = 0;
		bool is_skylander = false;
		uint8_t card_uid[NFC_UID_SIZE];
		while (true)
		{
			// Authorize Card Sector 0
			sky_auth auth_val = authSector0(nfc);

			if (auth_val == SKY_AUTH_SKYKEY)
			{
				std::cout << "\tAuthorized (Skylander KeyA)" << std::endl;
				is_skylander = true;
			}
			else if (auth_val == SKY_AUTH_DEFAULT)
			{
				std::cout << "\tAuthorized (Default KeyA)" << std::endl;
				is_skylander = false;
			}
			else
			{
				std::cout << "\tFailed" << std::endl << std::endl << "** ERROR: NFC Sector 0 Authentication Failed!" << std::endl << std::endl;
				return -1;
			}

			// Read Sector 0 for UID, which is used to compute access keys for all other sectors
			std::cout << "\tReading Sector 0: ";
			uint32_t retval = nfc.sectorRead(0, sky_card[0]);
			if (retval != NFC_OK)
			{
				std::cout << "Failed" << std::endl << std::endl << "** ERROR: Could not read Sector 0!" << std::endl << std::endl;
				return -1;
			}

			// Sector 0 Read success, get card UID
			memcpy(card_uid, sky_card[0].blk0, NFC_UID_SIZE);
			std::cout << "Done [UID=" << xk::skylanderNFC::toHexStr(card_uid, NFC_UID_SIZE) << "]" << std::endl;

			// Make sure if the target is a Skylander card - i.e file and card UIDs match
			if (memcmp(card_uid, file_uid, NFC_UID_SIZE) != 0)
			{
				// if UIDs do not match for Skylander card, do not overwrite a different Skylander model!
				if (is_skylander)
				{
					std::cout << "** Error: Can NOT overwrite a different Skylander!" << std::endl;
					return -1;
				}
				else
				{
					// If this is potentially a blank card, we can try overwrite UID with the Skylander UID
					if (retries < retries_max)
					{
						retries++;
						std::cout << "\tNo UID Match - Attempting to overwrite UID (" << retries << "/" << retries_max << ")" << std::endl;
						if (!writeSector0Block0(nfc, reader_name, sky_file[0]))
						{
							std::cout << "** ERROR - Failed to rewrite UID" << std::endl;
							return -1;
						}
					}
					else
					{
						std::cout << "** ERROR - UID Rewrite Failure!" << std::endl;
						return -1;
					}
				}
			}
			else
			{
				std::cout << "\tUID's match - proceeding with clone" << std::endl;
				break;
			}
		}

		std::cout << "STEP 4:\tCloning Skylander Sectors:" << std::endl;

		// Write all Sectors
		bool all_writes_ok = true;
		for (uint8_t sec_idx = 0; sec_idx < NFC_SECTORS_PER_CARD; sec_idx++)
		{
			uint32_t retval = NFC_ERROR;

			uint8_t block_start = 0;
			uint8_t block_count = NFC_BLOCKS_PER_SECTOR;

			std::cout << "[";

			// Sector Zero uses Skylander Key A
			if (sec_idx == 0)
			{
				uint8_t sky_key_a[] = SKYLANDER_BLOCK_0_KEY_A;
				memcpy(sky_file[0].key_A, sky_key_a, NFC_KEY_SIZE);
				sky_auth auth_blk0 = authSector0(nfc);

				switch (auth_blk0)
				{
					case SKY_AUTH_SKYKEY:
						block_count = block_start = 0; // do not write any blocks
						retval = NFC_OK;
						break;
					case SKY_AUTH_DEFAULT:
						std::cout << "d";
						retval = NFC_OK;
						break;
					default:
						retval = NFC_ERROR;
						break;
				}
			}
			else
			{
				// make Skylander Key for all other Sectors
				uint64_t sector_key = xk::skylanderNFC::makeSkyKey(sec_idx, file_uid);
				memcpy(sky_file[sec_idx].key_A, &sector_key, NFC_KEY_SIZE);
				retval = nfc.sectorAuthKeyA(sec_idx, sky_file[sec_idx].key_A);

				if (retval == NFC_OK)
				{
					block_start = 0;
					block_count = 3; // skip last block (Keys/Access Bits)
				}
				else
				{
					uint8_t default_key_a[] = NFC_DEFAULT_KEY_A;
					retval = nfc.sectorAuthKeyA(sec_idx, default_key_a);
					if (retval == NFC_OK)
						std::cout << "d";
				}
			}

			if (retval == NFC_OK)
				std::cout << "A";
			else
			{
				std::cout << "x";
				all_writes_ok = false;
			}

			retval = nfc.sectorRead(sec_idx, sky_card[sec_idx]);
			if (retval == NFC_OK)
				std::cout << "R";
			else
			{
				std::cout << "x";
				all_writes_ok = false;
			}

			// set card data
			memcpy(&sky_card[sec_idx], &sky_file[sec_idx], sizeof(xk::acr122u::NFC_Sector));

			retval = nfc.sectorWrite(sec_idx, sky_card[sec_idx], block_start, block_count);
			if (retval == NFC_OK)
				std::cout << "W" << (int)block_start << "_" << (int)block_count;
			else
			{
				std::cout << "x";
				all_writes_ok = false;
			}

			std::cout << "]" << std::flush;
		}
		std::cout << std::endl;

		if (!all_writes_ok)
		{
			std::cout << "** WARNING: Some sectors failed to write!" << std::endl;
		}

		// STEP 5: Verify written data
		std::cout << "STEP 5:\tVerifying Written Data:" << std::endl;
		xk::acr122u::NFC_Sector verify_card[NFC_SECTORS_PER_CARD];
		bool verification_ok = true;

		for (uint8_t sec_idx = 0; sec_idx < NFC_SECTORS_PER_CARD; sec_idx++)
		{
			std::cout << "[";
			uint32_t retval = NFC_ERROR;

			// Authenticate with Skylander key
			if (sec_idx == 0)
			{
				uint8_t sky_key_a[] = SKYLANDER_BLOCK_0_KEY_A;
				retval = nfc.sectorAuthKeyA(sec_idx, sky_key_a);
			}
			else
			{
				uint64_t sector_key = xk::skylanderNFC::makeSkyKey(sec_idx, file_uid);
				uint8_t key_bytes[NFC_KEY_SIZE];
				memcpy(key_bytes, &sector_key, NFC_KEY_SIZE);
				retval = nfc.sectorAuthKeyA(sec_idx, key_bytes);
			}

			if (retval != NFC_OK)
			{
				std::cout << "x]" << std::flush;
				verification_ok = false;
				continue;
			}

			std::cout << "A";

			retval = nfc.sectorRead(sec_idx, verify_card[sec_idx]);
			if (retval != NFC_OK)
			{
				std::cout << "x]" << std::flush;
				verification_ok = false;
				continue;
			}

			std::cout << "R";

			// Compare data blocks (skip key/access bits block for sectors > 0)
			bool sector_ok = true;
			uint8_t blocks_to_check = (sec_idx == 0) ? 3 : 3; // First 3 blocks in each sector

			for (uint8_t blk = 0; blk < blocks_to_check; blk++)
			{
				uint8_t* written = nullptr;
				uint8_t* readback = nullptr;

				switch (blk)
				{
					case 0:
						written = sky_file[sec_idx].blk0;
						readback = verify_card[sec_idx].blk0;
						break;
					case 1:
						written = sky_file[sec_idx].blk1;
						readback = verify_card[sec_idx].blk1;
						break;
					case 2:
						written = sky_file[sec_idx].blk2;
						readback = verify_card[sec_idx].blk2;
						break;
				}

				// Skip block 0 of sector 0 (UID block - read-only on real cards)
				if (sec_idx == 0 && blk == 0)
					continue;

				if (memcmp(written, readback, NFC_BLOCK_SIZE) != 0)
				{
					sector_ok = false;
					break;
				}
			}

			if (sector_ok)
				std::cout << "V]" << std::flush;
			else
			{
				std::cout << "x]" << std::flush;
				verification_ok = false;
			}
		}
		std::cout << std::endl;

		if (verification_ok)
			std::cout << "Clone Complete - Verification PASSED!" << std::endl;
		else
			std::cout << "** WARNING: Verification detected mismatches!" << std::endl;

		nfc.deviceDisconnect();
	}
	catch (xk::appError* e)
	{
		std::cout << "Error: " << e->what() << std::endl;
		delete e;
		return -1;
	}


	return 0;
}