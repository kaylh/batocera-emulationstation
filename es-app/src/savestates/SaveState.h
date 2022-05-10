#pragma once

#include <string>
#include "utils/TimeUtil.h"
#include "SaveStateConfig.h"

class FileData;

class SaveState
{
	friend class EmulatorSaveStateInfo;

protected:
	SaveState() { slot = -99; };
	
public:
	SaveState(int slotId);

	std::string				rom;
	EmulatorSaveStateInfo*	emulator;
	std::string				fileName;
	Utils::Time::DateTime	creationDate;
	int						slot;

	bool isSlotValid() const { return slot != -99; }
	std::string getScreenShot() const;

	bool copyToSlot(int slot, bool move = false) const;
	void remove() const;

public:
	virtual std::string makeStateFilename(int slot, bool fullPath = true) const;
	virtual std::string setupSaveState(FileData* game);
	virtual void onGameEnded(FileData* game);

private:
	std::string mAutoFileBackup;
	std::string mAutoImageBackup;
	std::string mNewSlotFile;
	std::string mNewSlotCheckSum;
};