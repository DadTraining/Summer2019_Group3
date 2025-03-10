
#include "MainMenu.h"
#include "ResourceManager.h"
#include "Map\House.h"
#include "Map\House.h"
#include "NPC\LakeNPC.h"
#include "Popup.h"

using namespace cocos2d;
using namespace std;

Scene* MainMenu::createScene()
{
	auto scene = Scene::create();
	auto layer = MainMenu::create();
	scene->addChild(layer);
	return scene;
}

bool MainMenu::init()
{
	if (!Layer::init())
	{
		return false;
	}
	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	backGround = ResourceManager::GetInstance()->GetSpriteById(134);
	backGround->removeFromParent();
	backGround->setAnchorPoint(Vec2(0, 0));
	backGround->setScale(visibleSize.width / backGround->getContentSize().width, visibleSize.height / backGround->getContentSize().height);
	addChild(backGround, -99);
	buttonPlay = ResourceManager::GetInstance()->GetButtonById(7);
	buttonPlay->removeFromParent();
	buttonPlay->setScale(1.2);
	buttonPlay->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 9));
	addChild(buttonPlay, -98);
	buttonPlay->addTouchEventListener([this](Ref* sender, ui::Widget::TouchEventType type)
	{
		if (type == Widget::TouchEventType::ENDED)
		{
			this->removeAllChildren();
			backGround->release();
			buttonPlay->release();
			buttonSetting->release();
			buttonAbout->release();
			buttonExit->release();
			Director::getInstance()->getRunningScene()->pause();
			Director::getInstance()->replaceScene(House::createScene());
		}
	});

	buttonSetting = ResourceManager::GetInstance()->GetButtonById(9);
	buttonSetting->removeFromParent();
	//buttonSetting->setScale();
	buttonSetting->setPosition(Vec2(visibleSize.width / 20, visibleSize.height / 10));
	addChild(buttonSetting, -1);
	buttonSetting->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type)
	{
		if (type == Widget::TouchEventType::ENDED)
		{
			UICustom::Popup *popup = UICustom::Popup::createSetting("Setting");
			popup->removeFromParent();
			this->addChild(popup);
		}
	});
	buttonAbout = ResourceManager::GetInstance()->GetButtonById(6);
	buttonAbout->removeFromParent();
	//buttonAbout->setScale(0.25f);
	buttonAbout->setPosition(Vec2(visibleSize.width * 19 / 20, visibleSize.height / 10));
	addChild(buttonAbout, -1);
	buttonAbout->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type)
	{
		if (type == Widget::TouchEventType::ENDED)
		{
			//Label *lbl = Label::createWithTTF("This game is created by TNVD team   Summer Internship 2019", "fonts/Marker Felt.ttf", 25);
			//lbl->setWidth(350);
			string index = "This game is created by TNVD team   Summer Internship 2019";
			UICustom::Popup *popup = UICustom::Popup::createAsMessage("About", index);
			this->addChild(popup);
		}
	});


	buttonExit = ResourceManager::GetInstance()->GetButtonById(10);
	buttonExit->removeFromParent();
	//buttonExit->setScale(0.1f);
	buttonExit->setPosition(Vec2(visibleSize.width * 19 / 20, visibleSize.height * 9 / 10));
	addChild(buttonExit, -1);
	buttonExit->setVisible(false);
	buttonExit->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type)
	{
		if (type == Widget::TouchEventType::ENDED)
		{
			Director::getInstance()->end();
			exit(0);
		}
	});
	return true;
}