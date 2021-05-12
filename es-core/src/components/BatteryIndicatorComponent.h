#pragma once

#ifndef ES_APP_COMPONENTS_BATTERYINDICATOR_COMPONENT_H
#define ES_APP_COMPONENTS_BATTERYINDICATOR_COMPONENT_H

#include "ControllerActivityComponent.h"

class BatteryIndicatorComponent : public ControllerActivityComponent
{
public:
	BatteryIndicatorComponent(Window* window);

	virtual std::string getTypeName() { return "batteryIndicator"; }

protected:
	virtual void init() override;
};

#endif // ES_APP_COMPONENTS_RATING_COMPONENT_H
