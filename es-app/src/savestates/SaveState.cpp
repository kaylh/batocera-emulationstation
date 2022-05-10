#include "SaveState.h"
#include "SystemData.h"
#include "FileData.h"
#include "utils/StringUtil.h"
#include "ApiSystem.h"
#include "FileData.h"
#include "SaveStateRepository.h"
#include "SystemConf.h"

SaveState::SaveState(int slotId)
{	
	slot = slotId;
}

std::string SaveState::makeStateFilename(int slot, bool fullPath) const
{
	std::string ret = this->rom + ".state" + (slot < 0 ? ".auto" : (slot == 0 ? "" : std::to_string(slot)));
	if (fullPath) 
		return Utils::FileSystem::combine(Utils::FileSystem::getParent(fileName), ret);

	return ret;
}

std::string SaveState::getScreenShot() const
{
	if (!fileName.empty() && Utils::FileSystem::exists(fileName + ".png"))
		return fileName + ".png";

	return "";
}

std::string SaveState::setupSaveState(FileData* game)
{
	if (game == nullptr)
		return "";
	
	if (this->emulator == nullptr)
	{
		auto emuls = SaveStateConfig::getEmulatorSaveStateInfos(game->getEmulator());
		if (emuls.size())
			this->emulator = emuls[0];
	}

	bool allowIncremental = this->emulator == nullptr || this->emulator->incremental;
	bool allowAutoSave = this->emulator != nullptr && this->emulator->autosave;
	bool incrementalSaveStates = allowIncremental && SystemConf::getIncrementalSaveStates();

	// We start games with new slots : If the users saves the game, we don't loose the previous save
	int nextSlot = SaveStateRepository::getNextFreeSlot(game, *this);

	// Game was launched normally. 
	if (!isSlotValid())
	{
		if (nextSlot > 0 && allowIncremental && !SystemConf::getIncrementalSaveStatesUseCurrentSlot())
		{
			// We start a game normally but there are saved games : Start game on next free slot to avoid loosing a saved game
			return "-state_slot " + std::to_string(nextSlot);
		}

		return "";
	}

	// Autosave is not supported by the emulator ?
	if (!allowAutoSave)
	{
		if (!fileName.empty())
			return "-state_slot " + std::to_string(incrementalSaveStates ? nextSlot : slot) + " -state_filename \"" + Utils::FileSystem::getFileName(fileName) + "\"";

		return "-state_slot " + std::to_string(incrementalSaveStates ? nextSlot : slot);
	}

	if (slot == -1) // Run current AutoSave
		return "-autosave 1 -state_slot " + std::to_string(nextSlot);

	if (slot == -2) // Run new game -> Force disable AutoSave (in case Autosave is enabled in settings)
		return "-autosave 0 -state_slot " + std::to_string(nextSlot);

	std::string cmd;	
	std::string path = Utils::FileSystem::getParent(fileName);
	
	if (incrementalSaveStates)
	{
		cmd = "-state_slot " + std::to_string(nextSlot); // slot

		// Run game, and activate AutoSave to load it
		if (!fileName.empty())
			cmd = cmd + " -autosave 1";
	}
	else
	{
		cmd = "-state_slot " + std::to_string(slot);

		// Run game, and activate AutoSave to load it
		if (!fileName.empty())
			cmd = cmd + " -autosave 1";
	}

	// Copy to state.auto file
	auto autoFilename = makeStateFilename(-1);
	if (Utils::FileSystem::exists(autoFilename))
	{
		Utils::FileSystem::removeFile(autoFilename + ".bak");
		Utils::FileSystem::renameFile(autoFilename, autoFilename + ".bak");				
	}

	// Copy to state.auto.png file
	auto autoImage = autoFilename + ".png";
	if (Utils::FileSystem::exists(autoImage))
	{
		Utils::FileSystem::removeFile(autoImage + ".bak");
		Utils::FileSystem::renameFile(autoImage, autoImage + ".bak");				
	}

	mAutoImageBackup = autoImage;
	mAutoFileBackup = autoFilename;

	if (!fileName.empty())
	{
		Utils::FileSystem::copyFile(fileName, autoFilename);

		if (incrementalSaveStates && nextSlot >= 0 && slot + 1 != nextSlot)
		{
			// Copy file to new slot, if the users want to reload the saved game in the slot directly from retroach
			mNewSlotFile = makeStateFilename(nextSlot);
			Utils::FileSystem::removeFile(mNewSlotFile);
			if (Utils::FileSystem::copyFile(fileName, mNewSlotFile))
				mNewSlotCheckSum = ApiSystem::getInstance()->getMD5(fileName, false);
		}
	}	
	
	return cmd;
}

void SaveState::onGameEnded(FileData* game)
{
	if (slot < 0)
		return;

	bool allowIncremental = this->emulator == nullptr || this->emulator->incremental;
	bool allowAutoSave = this->emulator != nullptr && this->emulator->autosave;

	if (allowAutoSave)
	{
		if (!mNewSlotCheckSum.empty() && Utils::FileSystem::exists(mNewSlotFile))
		{
			// Check if the file in the slot has changed. If it's the same, then delete it & clear the slot
			auto fileChecksum = ApiSystem::getInstance()->getMD5(mNewSlotFile, false);
			if (fileChecksum == mNewSlotCheckSum)
				Utils::FileSystem::removeFile(mNewSlotFile);
		}

		if (!mAutoFileBackup.empty())
		{
			Utils::FileSystem::removeFile(mAutoFileBackup);
			Utils::FileSystem::renameFile(mAutoFileBackup + ".bak", mAutoFileBackup);
		}

		if (!mAutoImageBackup.empty())
		{
			Utils::FileSystem::removeFile(mAutoImageBackup);
			Utils::FileSystem::renameFile(mAutoImageBackup + ".bak", mAutoImageBackup);
		}

		if (allowIncremental && SystemConf::getIncrementalSaveStates())
			SaveStateRepository::renumberSlots(game, *this);
	}
}

void SaveState::remove() const
{
	if (!isSlotValid())
		return;

	if (!fileName.empty())
		Utils::FileSystem::removeFile(fileName);

	if (!getScreenShot().empty())
		Utils::FileSystem::removeFile(getScreenShot());
}

bool SaveState::copyToSlot(int slot, bool move) const
{
	if (slot < 0)
		return false;

	if (!Utils::FileSystem::exists(fileName))
		return false;

	std::string destState = makeStateFilename(slot);
	
	if (move)
	{
		Utils::FileSystem::renameFile(fileName, destState);
		if (!getScreenShot().empty())
			Utils::FileSystem::renameFile(getScreenShot(), destState + ".png");

	}
	else
	{
		Utils::FileSystem::copyFile(fileName, destState);
		if (!getScreenShot().empty())
			Utils::FileSystem::copyFile(getScreenShot(), destState + ".png");
	}

	return true;
}