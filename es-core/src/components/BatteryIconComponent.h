#pragma once

#ifndef ES_CORE_COMPONENTS_BATTERYICON_COMPONENT_H
#define ES_CORE_COMPONENTS_BATTERYICON_COMPONENT_H

#include "GuiComponent.h"
#include "components/ImageComponent.h"

class Window;

class BatteryIconComponent : public ImageComponent
{
public:
	BatteryIconComponent(Window* window);

	virtual std::string getTypeName() { return "batteryIcon"; }

	virtual void update(int deltaTime);

private:
	int mNetworkCheckTime;

	std::string mIncharge;
	std::string mFull;
	std::string mAt75;
	std::string mAt50;
	std::string mAt25;
	std::string mEmpty;
};

#endif // ES_CORE_COMPONENTS_NETWORK_COMPONENT_H
