#include "components/BatteryIconComponent.h"
#include "Settings.h"
#include "platform.h"

#define UPDATE_NETWORK_DELAY	2000

BatteryIconComponent::BatteryIconComponent(Window* window) : ImageComponent(window)
{	
	mNetworkCheckTime = UPDATE_NETWORK_DELAY;

	mIncharge = ResourceManager::getInstance()->getResourcePath(":/battery/incharge.svg");
	mFull = ResourceManager::getInstance()->getResourcePath(":/battery/full.svg");
	mAt75 = ResourceManager::getInstance()->getResourcePath(":/battery/75.svg");
	mAt50 = ResourceManager::getInstance()->getResourcePath(":/battery/50.svg");
	mAt25 = ResourceManager::getInstance()->getResourcePath(":/battery/25.svg");
	mEmpty = ResourceManager::getInstance()->getResourcePath(":/battery/empty.svg");

	setVisible(Settings::getInstance()->getBool("ShowNetworkIndicator") && !queryIPAdress().empty());
}

void BatteryIconComponent::update(int deltaTime)
{
	ImageComponent::update(deltaTime);

	mNetworkCheckTime += deltaTime;
	if (mNetworkCheckTime >= UPDATE_NETWORK_DELAY)
	{		
		setVisible(Settings::getInstance()->getBool("ShowNetworkIndicator") && !queryIPAdress().empty());
		mNetworkCheckTime = 0;
	}
}
