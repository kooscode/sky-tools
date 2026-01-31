#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <vector>
#include <algorithm>
#include <cctype>

#include "cxxopts.hpp"
#include "appError.hpp"
#include "acr122u.hpp"
#include "skylanderNFC.hpp"
#include "skylanderDB.hpp"

// Generate a filename from Skylander info: name_type_element_UID.bin
std::string generateFilename(const xk::SkylanderInfo* info, uint16_t charId, const uint8_t* uid)
{
	std::string name, type, element;

	if (info)
	{
		name = info->name;
		type = xk::skylanderDB::getTypeString(info->type);
		element = info->element;
	}
	else
	{
		// Unknown Skylander - use ID
		char buf[32];
		snprintf(buf, sizeof(buf), "Unknown_%04X", charId);
		name = buf;
		type = "Skylander";
		element = "Unknown";
	}

	// Build filename: name_type_element_UID.bin
	std::string filename = name + "_" + type + "_" + element + "_" + xk::skylanderNFC::toHexStr(uid, NFC_UID_SIZE);

	// Convert to lowercase and replace spaces/special chars with underscores
	std::transform(filename.begin(), filename.end(), filename.begin(), [](unsigned char c) {
		if (c == ' ' || c == '-' || c == '.' || c == '\'')
			return (int)'_';
		return (int)std::tolower(c);
	});

	// Remove consecutive underscores
	std::string result;
	bool lastWasUnderscore = false;
	for (char c : filename)
	{
		if (c == '_')
		{
			if (!lastWasUnderscore)
				result += c;
			lastWasUnderscore = true;
		}
		else
		{
			result += c;
			lastWasUnderscore = false;
		}
	}

	return result + ".bin";
}

int main(int argc, char** argv)
{
	std::string sky_file_name = "";
	bool autoFilename = false;
	const uint8_t SKYKEY[] = SKYLANDER_BLOCK_0_KEY_A;
	const uint8_t DEFAULTKEY[] = NFC_DEFAULT_KEY_A;
	xk::acr122u::NFC_Sector card_sectors[NFC_SECTORS_PER_CARD];

	//setup cmdline options
	cxxopts::Options app_options("sky-dump", "Skylander NFC Dumper.");
	app_options.add_options()
		("f", "Target BIN filename (optional - auto-generates from Skylander info if not provided)", cxxopts::value<std::string>())
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
		return -1;
	}

	// print help if required
	if (app_params.count("help"))
	{
		std::cout << app_options.help() << std::endl;
		return 0;
	}

	// Check if filename provided
	if (app_params.count("f") > 0)
	{
		sky_file_name = app_params["f"].as<std::string>();
	}
	else
	{
		autoFilename = true;
	}

	//Connect to card reader and dump to file..
	try
	{
		xk::acr122u nfc;

		std::cout << "Looking for ACR122U NFC Reader/Writer..." << std::endl;
		std::string reader_name = nfc.findAndConnect();
		std::cout << "\tFound: " << reader_name << std::endl;
		std::cout << "\tConnected." << std::endl;

		// Auth using SKYKEY
		bool use_skykey = true;
		memcpy(card_sectors[0].key_A, SKYKEY, NFC_KEY_SIZE);
		uint32_t retval = nfc.sectorAuthKeyA(0, card_sectors[0].key_A);

		// If SKYKEY Key fails, retry using default Sector 0, KeyA = 0xFFFFFFFFFFFF
		if (retval == NFC_ERROR_KEYA_AUTH)
		{
			std::cout << "Failed." << std::endl;
			std::cout << "Authenticating Sector 0 (Using Default KeyA):" << std::endl << "\t";
			use_skykey = false;
			memcpy(card_sectors[0].key_A, DEFAULTKEY, NFC_KEY_SIZE);
			retval = nfc.sectorAuthKeyA(0, card_sectors[0].key_A);
		}

		// Block Zero Auth Failed
		if (retval != NFC_OK)
		{
			std::cout << "Failed." << std::endl << std::endl << "** ERROR: NFC Sector 0 Authentication Failed!" << std::endl << std::endl;
			return -1;
		}

		std::cout << "Sector 0 Access Authorized" << std::endl;

		// Read Sector 0 for UID, which is used to compute access keys for all other sectors
		std::cout << "Reading Sector 0:" << std::endl << "\t";
		retval = nfc.sectorRead(0, card_sectors[0]);
		if (retval != NFC_OK)
		{
			std::cout << "Failed." << std::endl << std::endl << "** ERROR: Could not read Sector 0!" << std::endl << std::endl;
			return -1;
		}

		// Sector 0 Read success, get card UID
		uint8_t card_uid[NFC_UID_SIZE];
		memcpy(card_uid, card_sectors[0].blk0, NFC_UID_SIZE);
		std::cout << "Done. [UID=" << xk::skylanderNFC::toHexStr(card_uid, NFC_UID_SIZE) << "]" << std::endl;

		// Identify Skylander
		uint8_t* card_data = reinterpret_cast<uint8_t*>(card_sectors);
		uint16_t charId = xk::skylanderDB::getCharacterId(card_data);
		const xk::SkylanderInfo* info = xk::skylanderDB::getInfo(charId);
		if (info)
		{
			std::cout << "Skylander: " << info->name << " (" << xk::skylanderDB::getTypeString(info->type) << ", " << info->element << ")" << std::endl;
		}
		else
		{
			std::cout << "Skylander: Unknown (ID: 0x" << std::hex << charId << std::dec << ")" << std::endl;
		}

		// Generate filename if not provided
		if (autoFilename)
		{
			sky_file_name = generateFilename(info, charId, card_uid);
		}

		// Read ALL Sectors
		std::cout << "Reading ALL Sectors:" << std::endl << "\t";
		bool all_sectors_ok = true;
		for (uint8_t sec_idx = 1; sec_idx < NFC_SECTORS_PER_CARD; sec_idx++)
		{
			// If this is a Skylander card, recompute sector keys
			if (use_skykey)
			{
				uint64_t sector_key = xk::skylanderNFC::makeSkyKey(sec_idx, card_uid);
				memcpy(card_sectors[sec_idx].key_A, &sector_key, NFC_KEY_SIZE);
			}

			// Auth + Read Card Sector
			retval = nfc.sectorAuthKeyA(sec_idx, card_sectors[sec_idx].key_A);
			if (retval == NFC_OK)
				retval = nfc.sectorRead(sec_idx, card_sectors[sec_idx]);

			if (retval == NFC_OK)
				std::cout << "." << std::flush;
			else
			{
				std::cout << "X" << std::flush;
				all_sectors_ok = false;
			}
		}
		std::cout << "\r\tDone.                                 " << std::endl;

		if (!all_sectors_ok)
		{
			std::cout << "** WARNING: Some sectors failed to read!" << std::endl;
		}

		// Dump to File
		std::cout << "Dumping to file " << sky_file_name << ":" << std::endl << "\t";
		char* pcard_sectors = static_cast<char*>(static_cast<void*>(&card_sectors));
		std::ofstream skyfile_out(sky_file_name.c_str(), std::ios::binary);
		skyfile_out.write(pcard_sectors, NFC_CARD_SIZE);
		skyfile_out.close();
		std::cout << "Done." << std::endl;

		// Disconnect from device
		std::cout << "Disconnect Reader/Writer:" << std::endl << "\t";
		nfc.deviceDisconnect();
		std::cout << "Disconnected." << std::endl;
	}
	catch (xk::appError* e)
	{
		std::cout << "Error: " << e->what() << std::endl;
		delete e;
		return -1;
	}

	return 0;
}