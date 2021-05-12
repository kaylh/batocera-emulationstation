#include "components/NetworkIconComponent.h"
#include "Settings.h"
#include "platform.h"

#define UPDATE_NETWORK_DELAY	2000

NetworkIconComponent::NetworkIconComponent(Window* window) : ImageComponent(window)
{	
	mNetworkCheckTime = UPDATE_NETWORK_DELAY;
	setVisible(Settings::getInstance()->getBool("ShowNetworkIndicator") && !queryIPAdress().empty());
}

void NetworkIconComponent::update(int deltaTime)
{
	ImageComponent::update(deltaTime);

	mNetworkCheckTime += deltaTime;
	if (mNetworkCheckTime >= UPDATE_NETWORK_DELAY)
	{		
		setVisible(Settings::getInstance()->getBool("ShowNetworkIndicator") && !queryIPAdress().empty());
		mNetworkCheckTime = 0;
	}
}
