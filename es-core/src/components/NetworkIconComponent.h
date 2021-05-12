#pragma once

#ifndef ES_CORE_COMPONENTS_NETWORK_COMPONENT_H
#define ES_CORE_COMPONENTS_NETWORK_COMPONENT_H

#include "GuiComponent.h"
#include "components/ImageComponent.h"

class Window;

class NetworkIconComponent : public ImageComponent
{
public:
	NetworkIconComponent(Window* window);

	virtual std::string getTypeName() { return "networkIcon"; }

	virtual void update(int deltaTime);

private:
	int mNetworkCheckTime;
};

#endif // ES_CORE_COMPONENTS_NETWORK_COMPONENT_H
