#pragma once

#include <string>
#include <vector>
#include <map>

#include <regex>

class SaveState;

class EmulatorSaveStateInfo
{
public:
	SaveState* createEmptySaveState();

	EmulatorSaveStateInfo()
	{
		autosave = false;
		slots = false;
		incremental = false;
	}

	std::regex  pattern;
	std::string emulator;	

	std::map<std::string, std::string> hashToName;

	bool autosave;
	bool slots;
	bool incremental;
};

class SaveStateConfig
{
public:
	static std::vector<EmulatorSaveStateInfo*> getEmulatorSaveStateInfos(const std::string& emulator);

private:
	static void loadSaveStateConfig();
	static std::vector<EmulatorSaveStateInfo*> mSaveStateInfo;
};
