#include "SaveStateRepository.h"
#include "SystemData.h"
#include "FileData.h"
#include "utils/StringUtil.h"
#include "Paths.h"
#include "Log.h"
#include <time.h>
#include <regex>

#if WIN32
#include "Win32ApiSystem.h"
#endif

SaveStateRepository::SaveStateRepository(SystemData* system)
{
	mSystem = system;	
	mSupportsSlots = false;
	mDefaultAutoSave = nullptr;
	mDefaultNewGame = new SaveState(-2);

	std::vector<EmulatorSaveStateInfo*> info;

	for (auto& emul : mSystem->getEmulators())
	{
		for (auto ei : SaveStateConfig::getEmulatorSaveStateInfos(emul.name))
		{
			if (ei->autosave)
			{
				mDefaultAutoSave = ei->createEmptySaveState();
				mDefaultAutoSave->slot = -1;
			}

			if (ei->slots)
				mSupportsSlots = true;

			mConfig.push_back(ei);
		}
	}

	refresh();
}

SaveStateRepository::~SaveStateRepository()
{
	if (mDefaultNewGame)
	{
		delete mDefaultNewGame;
		mDefaultNewGame = nullptr;
	}
	
	if (mDefaultAutoSave)
	{
		delete mDefaultAutoSave;
		mDefaultAutoSave = nullptr;
	}

	clear();
}

void SaveStateRepository::clear()
{
	for (auto item : mStates)
		for (auto state : item.second)
			delete state;

	mStates.clear();
}

std::string SaveStateRepository::getSavesPath()
{
	return Utils::FileSystem::combine(Paths::getSavesPath(), mSystem->getName());
}
	
void SaveStateRepository::refresh()
{
	clear();

	auto path = getSavesPath();
	if (!Utils::FileSystem::exists(path))
		return;

	auto files = Utils::FileSystem::getDirectoryFiles(path);
	if (!files.size())
		return;

	for (auto config : mConfig)
	{		
		for (auto file : files)
		{
			if (file.hidden || file.directory)
				continue;

			std::string fileName = Utils::FileSystem::getFileName(file.path);

			std::cmatch matches;
			if (!std::regex_match(fileName.c_str(), matches, config->pattern))
				continue;
			
			if (matches.size() < 2)
				continue;

			std::string romStem = matches[1].str();
			std::string slot = matches.size() < 3 ? "" : matches[2].str();

			SaveState* state = config->createEmptySaveState();
			state->rom = romStem;
			state->emulator = config;
			state->fileName = file.path;
			state->slot = slot == ".auto" ? -1 : Utils::String::toInteger(slot);

			if (!config->autosave && state->slot < 0)
			{
				delete state;
				continue;
			}

#if WIN32
			state->creationDate.setTime(file.lastWriteTime);
#else
			state->creationDate = Utils::FileSystem::getFileModificationDate(state->fileName);
#endif

			mStates[romStem].push_back(state);
		}
	}
}

bool SaveStateRepository::hasSaveStates(FileData* game)
{
	if (mStates.size())
	{
		if (game->getSourceFileData()->getSystem() != mSystem)
			return false;

		auto it = mStates.find(Utils::FileSystem::getStem(game->getPath()));
		if (it != mStates.cend())
			return true;

		auto hash = game->getMetadata(MetaDataId::CheevosHash);
		if (!hash.empty())
		{
			for (auto& emul : mSystem->getEmulators())
			{
				for (auto ei : SaveStateConfig::getEmulatorSaveStateInfos(emul.name))
				{
					auto itRom = ei->hashToName.find(hash);
					if (itRom != ei->hashToName.cend())
					{
						it = mStates.find(itRom->second);
						if (it != mStates.cend())
							return true;
					}
				}
			}
		}
	}

	return false;
}

VectorEx<SaveState*> SaveStateRepository::getSaveStates(FileData* game, EmulatorSaveStateInfo* emulatorInfo)
{
	VectorEx<SaveState*> ret;

	bool hasAutoSave = false;

	if (isEnabled(game) && game->getSourceFileData()->getSystem() == mSystem)
	{
		auto it = mStates.find(Utils::FileSystem::getStem(game->getPath()));
		if (it != mStates.cend())
		{
			for (auto item : it->second)
			{
				if (emulatorInfo == nullptr || item->emulator == emulatorInfo)
				{
					hasAutoSave |= (item->slot == -1);
					ret.push_back(item);
				}
			}
		}

		auto hash = game->getMetadata(MetaDataId::CheevosHash);
		if (!hash.empty())
		{
			for (auto& emul : mSystem->getEmulators())
			{
				for (auto ei : SaveStateConfig::getEmulatorSaveStateInfos(emul.name))
				{
					if (emulatorInfo != nullptr && ei != emulatorInfo)
						continue;

					auto itRom = ei->hashToName.find(hash);
					if (itRom != ei->hashToName.cend())
					{
						it = mStates.find(itRom->second);
						if (it != mStates.cend())
						{
							for (auto item : it->second)
							{
								hasAutoSave |= (item->slot == -1);
								ret.push_back(item);
							}
						}
					}
				}
			}
		}
	}

	if (!hasAutoSave && mConfig.any([](EmulatorSaveStateInfo* c) { return c->autosave; }))
	{

	}

	return ret;
}

bool SaveStateRepository::isEnabled(FileData* game)
{
//	if (!game->isFeatureSupported(EmulatorFeatures::autosave))
//		return false;

	auto system = game->getSourceFileData()->getSystem();
	if (system->hasPlatformId(PlatformIds::IMAGEVIEWER))
		return false;

	auto repo = system->getSaveStateRepository();
	if (repo == nullptr || repo->mConfig.size() == 0 || repo->getSavesPath().empty())
		return false;	

	return true;
}

int SaveStateRepository::getNextFreeSlot(FileData* game, const SaveState& state)
{
	if (!isEnabled(game))
		return -99;
	
	auto repo = game->getSourceFileData()->getSystem()->getSaveStateRepository();
	auto states = repo->getSaveStates(game, state.emulator);
	if (states.size() == 0)
		return 0;

	for (int i = 99999; i >= 0; i--)
	{
		auto it = std::find_if(states.cbegin(), states.cend(), [i, state](const SaveState* x) { return x->slot == i; });
		if (it != states.cend())
			return i + 1;
	}

	return -99;
}

void SaveStateRepository::renumberSlots(FileData* game, const SaveState& state)
{
	if (!isEnabled(game))
		return;

	auto repo = game->getSourceFileData()->getSystem()->getSaveStateRepository();	
	repo->refresh();

	auto states = repo->getSaveStates(game, state.emulator);
	if (states.size() == 0)
		return;

	std::sort(states.begin(), states.end(), [](const SaveState* file1, const SaveState* file2) { return file1->slot < file2->slot; });

	int slot = 0;

	for (auto state : states)
	{
		if (state->slot < 0)
			continue;

		if (state->slot != slot)
			state->copyToSlot(slot, true);
		
		slot++;
	}	
}
