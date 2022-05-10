#pragma once

#include <string>
#include <vector>
#include <map>

#include "SaveState.h"
#include "SaveStateConfig.h"
#include "utils/VectorEx.h"

class SystemData;
class FileData;

class SaveStateRepository
{
public:
	SaveStateRepository(SystemData* system);
	~SaveStateRepository();

	static bool isEnabled(FileData* game);

	static int	getNextFreeSlot(FileData* game, const SaveState& state);
	static void renumberSlots(FileData* game, const SaveState& state);

	bool hasSaveStates(FileData* game);

	VectorEx<SaveState*> getSaveStates(FileData* game, EmulatorSaveStateInfo* emulatorInfo = nullptr);

	std::string getSavesPath();

	void clear();
	void refresh();

	SaveState* getDefaultAutosave() { return mDefaultAutoSave; };
	SaveState* getDefaultNewGame() { return mDefaultNewGame; };

	bool getSupportsAutosave() { return mDefaultAutoSave != nullptr; };
	bool getSupportsSlots() { return mSupportsSlots; };

private:
	SystemData* mSystem;
	std::map<std::string, std::vector<SaveState*>> mStates;

	SaveState* mDefaultAutoSave;
	SaveState* mDefaultNewGame;

	bool mSupportsSlots;

	VectorEx<EmulatorSaveStateInfo*> mConfig;
};
