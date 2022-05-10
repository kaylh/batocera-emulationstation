#include "SaveStateConfig.h"
#include "Paths.h"
#include "Log.h"
#include "utils/StringUtil.h"
#include "utils/FileSystemUtil.h"
#include <pugixml/src/pugixml.hpp>
#include <mutex>
#include "SaveState.h"

static std::mutex _mutex;
std::vector<EmulatorSaveStateInfo*> SaveStateConfig::mSaveStateInfo;

SaveState* EmulatorSaveStateInfo::createEmptySaveState()
{
	SaveState* ret = new SaveState();
	ret->emulator = this;
	return ret;
}

void SaveStateConfig::loadSaveStateConfig()
{
	std::string path = Paths::getUserEmulationStationPath() + "/es_savestates.cfg";
	if (!Utils::FileSystem::exists(path))
		path = Paths::getEmulationStationPath() + "/es_savestates.cfg";

	if (!Utils::FileSystem::exists(path))
		return;

	pugi::xml_document doc;
	pugi::xml_parse_result res = doc.load_file(path.c_str());

	if (!res)
	{
		LOG(LogError) << "Could not parse es_savestates.cfg file!";
		LOG(LogError) << res.description();
		return;
	}

	pugi::xml_node savestates = doc.child("savestates");
	if (!savestates)
	{
		LOG(LogError) << "es_savestates.cfg is missing the <savestates> tag!";
		return;
	}

	for (pugi::xml_node emulatorNode = savestates.first_child(); emulatorNode; emulatorNode = emulatorNode.next_sibling())
	{
		std::string name = emulatorNode.name();
		if (name == "emulator")
		{
			if (!emulatorNode.attribute("name") || !emulatorNode.attribute("pattern"))
				continue;
				
			EmulatorSaveStateInfo* info = new EmulatorSaveStateInfo();
			info->emulator = emulatorNode.attribute("name").value();
			info->pattern = std::regex(emulatorNode.attribute("pattern").value());

			if (emulatorNode.attribute("features"))
			{
				std::string features = emulatorNode.attribute("features").value();
				info->autosave = features.find("autosave") != std::string::npos;
				info->slots = features.find("slots") != std::string::npos;
				info->incremental = features.find("incremental") != std::string::npos;
			}

			for (pugi::xml_node romNode = emulatorNode.child("rom"); romNode; romNode = romNode.next_sibling("rom"))
			{
				if (!romNode.attribute("hash") || !romNode.attribute("name"))
					continue;
				
				std::string hash = romNode.attribute("hash").value();
				std::string romName = romNode.attribute("name").value();
				info->hashToName[hash] = romName;
			}

			mSaveStateInfo.push_back(info);			
		}
	}
}

std::vector<EmulatorSaveStateInfo*> SaveStateConfig::getEmulatorSaveStateInfos(const std::string& emulator)
{
	// Thread Locked block
	{
		std::unique_lock<std::mutex> lock(_mutex);

		if (!mSaveStateInfo.size())
		{
			loadSaveStateConfig();
			if (!mSaveStateInfo.size())
			{
				EmulatorSaveStateInfo* info = new EmulatorSaveStateInfo();
				info->emulator = "libretro";
				info->pattern = "^(.*)\\.state([0-9]*|.auto)$";
				info->autosave = true;
				info->incremental = true;
				info->slots = true;				
				mSaveStateInfo.push_back(info);
			}
		}
	}

	std::vector<EmulatorSaveStateInfo*> ret;

	auto emulatorName = emulator;
	if (emulatorName == "angle" || Utils::String::startsWith(emulatorName, "lr-"))
		emulatorName == "libretro";

	for (auto it : mSaveStateInfo)
		if (it->emulator == emulatorName)
			ret.push_back(it);

	return ret;
}
