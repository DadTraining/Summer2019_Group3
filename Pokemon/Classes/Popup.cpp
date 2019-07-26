#include "ui\UITabControl.h"
#include "Popup.h"
#include "ResourceManager.h"
#include "Bag.h"
#include "Pokemon\Charizard.h"
#include "Pokemon\Charmeleon.h"
#include "Pokemon\Chikorita.h"
#include "Pokemon\Squirtle.h"
#include "Pokemon.h"
#include "Buttons.h"
USING_NS_CC;

using namespace ui;

namespace  {
    cocos2d::Size CONFIRM_DIALOGUE_SIZE_OFFSET = Size(100,150);
    const float ANIMATION_TIME = 0.15f;
    const float FADE_RATIO = 150;
}

namespace FONT {
    const float LABEL_OFFSET = 50;
    const float DESCRIPTION_TEXT_SIZE = 20;
    const float TITLE_TEXT_SIZE = 30;
    const char *GAME_FONT = "res/UI/Label/MarkerFelt.ttf";
    const float LABEL_STROKE = 4;
}

namespace IMAGEPATH {
    const char *OK_BUTTON = "res/UI/Button/Ok.png";
    const char *OK_BUTTON_PRESSED = "res/UI/Button/OkPressed.png";
    const char *CANCEL_BUTTON = "res/UI/Button/Cancel.png";
    const char *CANCEL_BUTTON_PRESSED = "res/UI/Button/CancelPressed.png";
    const char *CLOSE_BUTTON = "res/UI/Button/close.png";
    const char *BACKGROUND_IMAGE = "res/Background/popUpBase.png";
	const char *SOUND_BUTTON = "res/UI/Checkbox/1.png";
	const char *SOUND_BUTTON_CHECK = "res/UI/Checkbox/2.png";
	const char *MUSIC_BUTTON = "res/UI/Checkbox/3.png";
	const char *MUSIC_BUTTON_CHECK = "res/UI/Checkbox/4.png";
}
namespace UICustom
{
	PopupDelegates *PopupDelegates::create()
	{
		PopupDelegates *node = new (std::nothrow)PopupDelegates();
		if (node && node->init())
		{
			node->autorelease();
			return node;
		}
		CC_SAFE_DELETE(node);
		return nullptr;
	}

	bool PopupDelegates::init()
	{
		Size winSize = Director::getInstance()->getWinSize();

		if (!LayerRadialGradient::initWithColor(Color4B(0, 0, 0, 0), Color4B(0, 0, 0, FADE_RATIO), winSize.width / 1.7f, winSize / 2, 0.075f))
		{
			return false;
		}
		this->setOpacity(0);
		show(true);
		//this->setUpTouches();


		return true;
	}

	void PopupDelegates::show(const bool animated)
	{
		if (animated) {
			this->runAction(FadeTo::create(ANIMATION_TIME, FADE_RATIO));
		}
		else {
			this->setOpacity(FADE_RATIO);
		}
	}
	void PopupDelegates::dismiss(const bool animated)
	{
		if (animated) {
			this->runAction(Sequence::create(FadeTo::create(ANIMATION_TIME, 0), RemoveSelf::create(), NULL));
		}
		else {
			this->removeFromParentAndCleanup(true);
		}
	}

	void PopupDelegates::setUpTouches()
	{

		auto listener = EventListenerTouchOneByOne::create();
		listener->setSwallowTouches(true);
		listener->onTouchBegan = [=](Touch* touch, Event* event) {
			if (_bg)
			{
				if (!_bg->getBoundingBox().containsPoint(this->convertToNodeSpace(touch->getLocation())))
				{
					this->dismiss(true);
				}
			}
			else
			{
				this->dismiss(true);
			}
			return true;
		};
		_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
	}
	Popup *Popup::createAsMessage(const std::string &title, const std::string &msg)
	{
		return createAsConfirmDialogue(title, msg, NULL);
	}

	Popup *Popup::createAsConfirmDialogue(const std::string &title, const std::string &msg, const std::function<void()> &YesFunc)
	{
		return create(title, msg, NULL, YesFunc);
	}
	Popup *Popup::createSetting(const std::string &title)
	{
		Popup *node = new (std::nothrow)Popup();
		Size winSize = Director::getInstance()->getWinSize();
		if (node && node->init())
		{
			Layout *layoutsetting = Layout::create();
			layoutsetting->setAnchorPoint(Vec2(0.5, 0.5));
			layoutsetting->setContentSize(Size(300, 100));
			layoutsetting->setPosition(winSize / 2);
			layoutsetting->removeFromParent();
			node->addChild(layoutsetting, 3);
			CheckBox *checkboxSound = CheckBox::create(IMAGEPATH::SOUND_BUTTON, IMAGEPATH::SOUND_BUTTON_CHECK);
			checkboxSound->setPosition(Vec2(layoutsetting->getContentSize().width / 4, layoutsetting->getContentSize().height / 2));
			checkboxSound->setScale(2);
			CheckBox *checkboxMusic = CheckBox::create(IMAGEPATH::MUSIC_BUTTON, IMAGEPATH::MUSIC_BUTTON_CHECK);
			checkboxMusic->setPosition((Vec2(layoutsetting->getContentSize().width * 3 / 4, layoutsetting->getContentSize().height / 2)));
			checkboxMusic->setScale(2);
			layoutsetting->addChild(checkboxMusic, 5);
			layoutsetting->addChild(checkboxSound, 5);
			MenuItemImage *yesButton = MenuItemImage::create(IMAGEPATH::OK_BUTTON, IMAGEPATH::OK_BUTTON_PRESSED, [=](Ref *sender) {
				node->dismiss(true);
			});

			MenuItemImage *noButton = MenuItemImage::create(IMAGEPATH::CANCEL_BUTTON, IMAGEPATH::CANCEL_BUTTON_PRESSED, [node](Ref *sender) {
				node->dismiss(true);
			});


			Menu *menu = Menu::create(yesButton, noButton, NULL);
			node->addChild(menu, 2);
			menu->setPosition(winSize.width / 2, winSize.height / 2 - layoutsetting->getContentSize().height / 2 - 75);
			menu->alignItemsHorizontallyWithPadding(FONT::LABEL_OFFSET / 2);
			CONFIRM_DIALOGUE_SIZE_OFFSET = Size(CONFIRM_DIALOGUE_SIZE_OFFSET.width, 250);
			//node->addChild(menuSetting, 2);
			menu->removeFromParent();
			node->addChild(menu, 2);
			node->initBg(layoutsetting->getContentSize() + CONFIRM_DIALOGUE_SIZE_OFFSET, title);
			node->autorelease();
			return node;
		}
	}
	Popup *Popup::createBag(const std::string &title)
	{
		Popup *node = new (std::nothrow)Popup();
		Size winSize = Director::getInstance()->getWinSize();
		if (node && node->init())
		{
			auto tab = TabControl::create();
			tab->setContentSize(Size(300, 100));
			tab->setHeaderHeight(20);
			tab->setHeaderWidth(100);
			tab->setHeaderDockPlace(TabControl::Dock::TOP);
			tab->setPosition(winSize / 2);
			node->addChild(tab, 101);
			auto headerMP = TabHeader::create();
			headerMP->setTitleText("My Pokemon");
			headerMP->setTitleColor(Color4B(0, 0, 0, 255));
			headerMP->loadTextureBackGround("res/Background/2.png");
			headerMP->setTitleFontName(FONT::GAME_FONT);

			auto headerPO = TabHeader::create();
			headerPO->loadTextureBackGround("res/Background/2.png");
			headerPO->setTitleText("Box Pokemon");
			headerPO->setTitleColor(Color4B(0, 0, 0, 255));
			headerPO->setTitleFontName(FONT::GAME_FONT);

			auto headerMI = TabHeader::create();
			headerMI->loadTextureBackGround("res/Background/2.png");
			headerMI->setTitleText("My Item");
			headerMI->setTitleColor(Color4B(0, 0, 0, 255));
			headerMI->setTitleFontName(FONT::GAME_FONT);

			auto containerMP = Layout::create();
#pragma region Tab1
			containerMP->setTouchEnabled(false);
			auto listView = ui::ListView::create();
			listView->setDirection(ui::ScrollView::Direction::HORIZONTAL);
			listView->setAnchorPoint(Vec2(0.5, 0.5));
			listView->setContentSize(Size(280, 140));
			listView->setScale(0.8);
			listView->setPosition(Vec2(tab->getContentSize().width / 2, 0));
			listView->setClippingEnabled(true);
			containerMP->addChild(listView,2000);
			listView->addEventListener((ListView::ccListViewCallback)CC_CALLBACK_2(Popup::SelectedItemEvent, node));
			auto list = Bag::GetInstance()->GetListPokemon();
			auto sizeOfPokemon = list.size();
			for (int i = 0; i < 6; i++)
			{
				ui::Button *button = ResourceManager::GetInstance()->GetButtonById(12);
				if (i<sizeOfPokemon)
				{
					string name = list.at(i)->GetName();
					string level = "Level:" + to_string(list.at(i)->GetLevel());
					Sprite *sprite = Sprite::create("res/Animation/" + name + "/front/0.png");
					sprite->setPosition(button->getPosition().x + listView->getContentSize().width* (i * 2 + 1) / 4,
						button->getPosition().y + listView->getContentSize().height / 2);
					Label* labelName = ResourceManager::GetInstance()->GetLabelById(0);
					labelName->setString(name);
					labelName->setColor(Color3B(0, 0, 0));
					labelName->setPosition(button->getPosition().x + listView->getContentSize().width* (i * 2 + 1) / 4,
						button->getPosition().y + listView->getContentSize().height * 4 / 5);
					Label* labelLv = ResourceManager::GetInstance()->GetLabelById(0);
					labelLv->setString(level);
					labelLv->setColor(Color3B(0, 0, 0));
					labelLv->setPosition(button->getPosition().x + listView->getContentSize().width* (i * 2 + 1) / 4,
						button->getPosition().y + listView->getContentSize().height * 1 / 5);
					listView->addChild(sprite, 202);
					listView->addChild(labelName, 202);
					listView->addChild(labelLv, 202);
				}
				listView->pushBackCustomItem(button);
			}

#pragma endregion
			auto containerPO = Layout::create();
#pragma region Tab2
			auto listViewPO = ui::ListView::create();
			listViewPO->setDirection(ui::ScrollView::Direction::HORIZONTAL);
			listViewPO->setAnchorPoint(Vec2(0.5, 0.5));
			listViewPO->setContentSize(Size(280, 140));
			listViewPO->setScale(0.8);
			CCLOG("%f", listViewPO->getContentSize().width);
			listViewPO->setGlobalZOrder(200);
			listViewPO->setPosition(Vec2(tab->getContentSize().width / 2, 0));
			listViewPO->setClippingEnabled(true);
			containerPO->addChild(listViewPO);
			int sizeOfPokemonOver = Bag::GetInstance()->GetListPokemonOver().size();
			auto listOver = Bag::GetInstance()->GetListPokemonOver();
			for (int i = 0; i < 10; i++)
			{
				ui::Button *buttonOver = ResourceManager::GetInstance()->GetButtonById(12);
				buttonOver->setTag(i);
				if (i < sizeOfPokemonOver)
				{
					string name = listOver.at(i)->GetName();
					string level = "Level:" + to_string(list.at(i)->GetLevel());
					Sprite *sprite = Sprite::create("res/Animation/" + name + "/front/0.png");
					sprite->setTag(i);
					sprite->setPosition(buttonOver->getPosition().x + listViewPO->getContentSize().width* (i * 2 + 1) / 4,
						buttonOver->getPosition().y + listViewPO->getContentSize().height / 2);
					Label* labelName = ResourceManager::GetInstance()->GetLabelById(0);
					labelName->setString(name);
					labelName->setColor(Color3B(0, 0, 0));
					labelName->setPosition(buttonOver->getPosition().x + listView->getContentSize().width* (i * 2 + 1) / 4,
						buttonOver->getPosition().y + listView->getContentSize().height * 4 / 5);
					Label* labelLv = ResourceManager::GetInstance()->GetLabelById(0);
					labelLv->setString(level);
					labelLv->setColor(Color3B(0, 0, 0));
					labelLv->setPosition(buttonOver->getPosition().x + listView->getContentSize().width* (i * 2 + 1) / 4,
						buttonOver->getPosition().y + listView->getContentSize().height * 1 / 5);
					listViewPO->addChild(labelName, 202);
					listViewPO->addChild(labelLv, 202);
					listViewPO->addChild(sprite, 202);
				}
				listViewPO->pushBackCustomItem(buttonOver);
			}
#pragma endregion
			auto containerMI = Layout::create();
#pragma region Tab3
			auto listViewMI = ui::ListView::create();
			listViewMI->setDirection(ui::ScrollView::Direction::HORIZONTAL);
			listViewMI->setAnchorPoint(Vec2(0.5, 0.5));
			listViewMI->setContentSize(Size(280, 140));
			listViewMI->setScale(0.8);
			CCLOG("%f", listViewPO->getContentSize().width);
			listViewMI->setGlobalZOrder(200);
			listViewMI->setPosition(Vec2(tab->getContentSize().width / 2, 0));
			listViewMI->setClippingEnabled(true);
			containerMI->addChild(listViewMI);
			int sizeOfItem = Bag::GetInstance()->GetListItem().size();
			auto listItem = Bag::GetInstance()->GetListItem();
			for (int i = 0; i < sizeOfItem; i++)
			{
				ui::Button *buttonItem = ResourceManager::GetInstance()->GetButtonById(12);
				buttonItem->setTag(i);
				string id = to_string(listItem.at(i)->GetId());
				string name = listItem.at(i)->GetName();
				string amount = "x" + to_string(listItem.at(i)->GetNumber());
				Sprite *sprite = ResourceManager::GetInstance()->GetSpriteById(137 + i);
				sprite->setTag(i);
				sprite->setScale(2.5);
				sprite->setPosition(buttonItem->getPosition().x + listViewMI->getContentSize().width* (i * 2 + 1) / 4,
					buttonItem->getPosition().y + listViewMI->getContentSize().height / 2);
				Label* labelName = ResourceManager::GetInstance()->GetLabelById(0);
				labelName->setString(name);
				labelName->setColor(Color3B(0, 0, 0));
				labelName->setPosition(buttonItem->getPosition().x + listViewMI->getContentSize().width* (i * 2 + 1) / 4,
					buttonItem->getPosition().y + listViewMI->getContentSize().height * 4 / 5);
				Label* labelNumber = ResourceManager::GetInstance()->GetLabelById(0);
				labelNumber->setString(amount);
				labelNumber->setColor(Color3B(0, 0, 0));
				labelNumber->setPosition(buttonItem->getPosition().x + listViewMI->getContentSize().width* (i * 2 + 1) / 4,
					buttonItem->getPosition().y + listViewMI->getContentSize().height * 1 / 5);
				listViewMI->addChild(labelName, 202);
				listViewMI->addChild(labelNumber, 202);
				listViewMI->addChild(sprite, 202);
				listViewMI->pushBackCustomItem(buttonItem);
			}
#pragma endregion
			tab->insertTab(0, headerMP, containerMP);
			tab->insertTab(1, headerPO, containerPO);
			tab->insertTab(2, headerMI, containerMI);
			tab->setSelectTab(0);
			CC_SAFE_RETAIN(tab);
			CC_SAFE_RETAIN(headerMP);
			CC_SAFE_RETAIN(headerPO);
			CC_SAFE_RETAIN(headerMI);

			MenuItemImage *noButton = MenuItemImage::create(IMAGEPATH::CANCEL_BUTTON, IMAGEPATH::CANCEL_BUTTON_PRESSED, [node](Ref *sender) {
				node->dismiss(true);
				Buttons::GetIntance()->GetButtonBag()->setTouchEnabled(true);
			});


			Menu *menu = Menu::create(noButton, NULL);
			node->addChild(menu, 2);
			menu->setPosition(winSize.width / 2, winSize.height / 2 - tab->getContentSize().height / 2 - 100);
			menu->alignItemsHorizontallyWithPadding(FONT::LABEL_OFFSET / 2);
			CONFIRM_DIALOGUE_SIZE_OFFSET = Size(CONFIRM_DIALOGUE_SIZE_OFFSET.width, 250);
			menu->removeFromParent();
			node->addChild(menu, 2);
			node->initBg(tab->getContentSize() + CONFIRM_DIALOGUE_SIZE_OFFSET, title);
			node->autorelease();
			return node;
		}
		CC_SAFE_DELETE(node);
		return nullptr;
	}
	Popup * Popup::ChoosePokemon()
	{
		Popup *node = new (std::nothrow)Popup();
		Size winSize = Director::getInstance()->getWinSize();
		if (node && node->init())
		{			
			Button *buttonCharmander = ResourceManager::GetInstance()->GetButtonById(12);
			buttonCharmander->setPosition(Vec2(winSize.width/2,winSize.height/2-buttonCharmander->getContentSize().height/4));
			node->addChild(buttonCharmander, 100);
			
			Button *buttonSquirtle = ResourceManager::GetInstance()->GetButtonById(12);
			buttonSquirtle->setPosition(Vec2(buttonCharmander->getPosition().x - buttonCharmander->getContentSize().width -10, buttonCharmander->getPosition().y));
			node->addChild(buttonSquirtle, 100);
			
			Button *buttonChikorita = ResourceManager::GetInstance()->GetButtonById(12);
			buttonChikorita->setPosition(Vec2(buttonCharmander->getPosition().x + buttonCharmander->getContentSize().width + 10, buttonCharmander->getPosition().y));
			node->addChild(buttonChikorita, 100);

			MenuItemImage *noButton = MenuItemImage::create(IMAGEPATH::CANCEL_BUTTON, IMAGEPATH::CANCEL_BUTTON_PRESSED, [node](Ref *sender) {
				node->dismiss(true);
				Buttons::GetIntance()->GetButtonBag()->setTouchEnabled(true);
			});
			
			Pokemon *charmander = new Charmander();
			Sprite *spriteCharmander = charmander->GetSpriteFront();
			spriteCharmander->setScale(2);
			spriteCharmander->setAnchorPoint(Vec2(0.5, 0));
			spriteCharmander->setPosition(buttonCharmander->getContentSize().width/2, buttonCharmander->getContentSize().height / 4);
			buttonCharmander->addChild(spriteCharmander,102);
			string nameCharmander = "Charmander - Fire";
			Label *labelCharmander = ResourceManager::GetInstance()->GetLabelById(0);
			labelCharmander->setString(nameCharmander);
			labelCharmander->setColor(Color3B(0, 0, 0));
			labelCharmander->setPosition(buttonCharmander->getContentSize().width / 2, buttonCharmander->getContentSize().height *5/6);
			buttonCharmander->addChild(labelCharmander,102);
			
			Squirtle *squirtle = new Squirtle();
			Sprite *spriteSquirtle = squirtle->GetSpriteFront();
			spriteSquirtle->setScale(2);
			spriteSquirtle->setAnchorPoint(Vec2(0.5, 0));
			spriteSquirtle->setPosition(buttonCharmander->getContentSize().width / 2 , buttonCharmander->getContentSize().height / 4);
			buttonSquirtle->addChild(spriteSquirtle, 102);
			string nameSquirtle = "Squirtle - Water";
			Label *labelSquirtle = ResourceManager::GetInstance()->GetLabelById(0);
			labelSquirtle->setString(nameSquirtle);
			labelSquirtle->setColor(Color3B(0, 0, 0));
			labelSquirtle->setPosition(buttonCharmander->getContentSize().width / 2, buttonCharmander->getContentSize().height * 5 / 6);
			buttonSquirtle->addChild(labelSquirtle, 102);
			
			Chikorita *chikorita = new Chikorita();
			Sprite *spriteChikorita = chikorita->GetSpriteFront();
			spriteChikorita->setScale(2);
			spriteChikorita->setAnchorPoint(Vec2(0.5, 0));
			spriteChikorita->setPosition(buttonCharmander->getContentSize().width / 2, buttonCharmander->getContentSize().height / 4);
			buttonChikorita->addChild(spriteChikorita, 102);
			string nameChikorita = "Chikorita - Grass";
			Label *labelChikorita = ResourceManager::GetInstance()->GetLabelById(0);
			labelChikorita->setString(nameChikorita);
			labelChikorita->setColor(Color3B(0, 0, 0));
			labelChikorita->setPosition(buttonCharmander->getContentSize().width / 2, buttonCharmander->getContentSize().height * 5 / 6);
			buttonChikorita->addChild(labelChikorita, 102);
			
			Label *labelPokemon = ResourceManager::GetInstance()->GetLabelById(0);
			labelPokemon->setColor(Color3B(0, 0, 0));
			labelPokemon->setPosition(Vec2(winSize.width / 2, winSize.height * 5 / 8));
			node->addChild(labelPokemon, 100);
			
			buttonCharmander->addTouchEventListener([labelPokemon,buttonCharmander,buttonChikorita,buttonSquirtle, charmander, winSize,node](Ref* sender, Widget::TouchEventType type)
			{
				if (type == Widget::TouchEventType::ENDED)
				{
					Bag::GetInstance()->AddPokemon(charmander);
					labelPokemon->setString("Congratulations. Charmander will be your partner");
					buttonCharmander->setTouchEnabled(false);
					buttonChikorita->setTouchEnabled(false);
					buttonSquirtle->setTouchEnabled(false);
				}
			});

			buttonChikorita->addTouchEventListener([labelPokemon, buttonCharmander, buttonChikorita, buttonSquirtle, chikorita, winSize, node](Ref* sender, Widget::TouchEventType type)
			{
				if (type == Widget::TouchEventType::ENDED)
				{
					Bag::GetInstance()->AddPokemon(chikorita);
					labelPokemon->setString("Congratulations. Chikorita will be your partner");
					buttonCharmander->setTouchEnabled(false);
					buttonChikorita->setTouchEnabled(false);
					buttonSquirtle->setTouchEnabled(false);
				}
			});

			buttonSquirtle->addTouchEventListener([labelPokemon, buttonCharmander, buttonChikorita, buttonSquirtle, squirtle, winSize, node](Ref* sender, Widget::TouchEventType type)
			{
				if (type == Widget::TouchEventType::ENDED)
				{
					Bag::GetInstance()->AddPokemon(squirtle);
					labelPokemon->setString("Congratulations. Squirtle will be your partner");
					buttonCharmander->setTouchEnabled(false);
					buttonChikorita->setTouchEnabled(false);
					buttonSquirtle->setTouchEnabled(false);
				}
			});

			Menu *menu = Menu::create(noButton, NULL);
			node->addChild(menu, 2);
			menu->setPosition(winSize.width / 2, winSize.height / 2 - buttonCharmander->getContentSize().height / 1.1);
			menu->alignItemsHorizontallyWithPadding(FONT::LABEL_OFFSET / 2);
			CONFIRM_DIALOGUE_SIZE_OFFSET = Size(CONFIRM_DIALOGUE_SIZE_OFFSET.width, 180);
			node->initBg(Size(buttonCharmander->getContentSize().width*3 + CONFIRM_DIALOGUE_SIZE_OFFSET.width,
				buttonCharmander->getContentSize().height + CONFIRM_DIALOGUE_SIZE_OFFSET.height), "Choose the starting pokemon");
			node->autorelease();
			return node;
		}
		CC_SAFE_DELETE(node);
		return nullptr;
	}
	Popup * Popup::CreateBagInBattle()
	{
		Popup *node = new (std::nothrow)Popup();
		Size winSize = Director::getInstance()->getWinSize();
		if (node && node->init())
		{
			auto listViewPokemon = ui::ListView::create();
			listViewPokemon->setDirection(ui::ScrollView::Direction::HORIZONTAL);
			listViewPokemon->setAnchorPoint(Vec2(0.5, 0.5));
			listViewPokemon->setContentSize(Size(280, 140));
			listViewPokemon->setScale(0.8);
			listViewPokemon->setPosition(Vec2(winSize.width/2,winSize.height/2-listViewPokemon->getContentSize().height/4));
			listViewPokemon->setClippingEnabled(true);
			MenuItemImage *noButton = MenuItemImage::create(IMAGEPATH::CANCEL_BUTTON, IMAGEPATH::CANCEL_BUTTON_PRESSED, [node](Ref *sender) {
				node->dismiss(true);
				Buttons::GetIntance()->GetButtonBag()->setTouchEnabled(true);
			});
			node->addChild(listViewPokemon, 200);
			int sizeofpokemon = Bag::GetInstance()->GetListPokemon().size();
			auto list = Bag::GetInstance()->GetListPokemon();
			for (int i = 0; i < 6; i++)
			{
				ui::Button *button = ResourceManager::GetInstance()->GetButtonById(12);
				button->setTag(i);
				if (i < sizeofpokemon)
				{
					string name = list.at(i)->GetName();
					//int type = list.at(i)->GetType();
					string percentHP = to_string(list.at(i)->GetCurrentHP() *100 / list.at(i)->GetMaxHP());
					string level = "Level:" + to_string(list.at(i)->GetLevel());
					string hp = "HP: " + percentHP + " %";
					Sprite *sprite = Sprite::create("res/Animation/" + name + "/front/0.png");
					sprite->setTag(i);
					sprite->setPosition(button->getPosition().x + listViewPokemon->getContentSize().width* (i * 2 + 1) / 4,
						button->getPosition().y + listViewPokemon->getContentSize().height / 2 + 10);
					Label* labelName = ResourceManager::GetInstance()->GetLabelById(0);
					labelName->setString(name);
					labelName->setColor(Color3B(0, 0, 0));
					labelName->setPosition(button->getPosition().x + listViewPokemon->getContentSize().width* (i * 2 + 1) / 4,
						button->getPosition().y + listViewPokemon->getContentSize().height * 4 / 5 + 10);
					Label* labelLv = ResourceManager::GetInstance()->GetLabelById(0);
					labelLv->setString(level);
					labelLv->setColor(Color3B(0, 0, 0));
					labelLv->setPosition(button->getPosition().x + listViewPokemon->getContentSize().width* (i * 2 + 1) / 4,
						button->getPosition().y + listViewPokemon->getContentSize().height * 1 / 5 + 15);
					Label* HP = ResourceManager::GetInstance()->GetLabelById(0);
					HP->setString(hp);
					HP->setColor(Color3B(0, 0, 0));
					HP->setPosition(button->getPosition().x + listViewPokemon->getContentSize().width* (i * 2 + 1) / 4,
						button->getPosition().y + listViewPokemon->getContentSize().height * 1 / 5 - 10);

					listViewPokemon->addChild(sprite, 202);
					listViewPokemon->addChild(labelName, 202);
					listViewPokemon->addChild(labelLv, 202);
					listViewPokemon->addChild(HP, 202);
				}
				listViewPokemon->pushBackCustomItem(button);
			}
			Menu *menu = Menu::create(noButton, NULL);
			node->addChild(menu, 2);
			menu->setPosition(winSize.width / 2, winSize.height / 2 - listViewPokemon->getContentSize().height / 2 - 75);
			menu->alignItemsHorizontallyWithPadding(FONT::LABEL_OFFSET / 2);
			CONFIRM_DIALOGUE_SIZE_OFFSET = Size(CONFIRM_DIALOGUE_SIZE_OFFSET.width, 150);
			node->initBg(listViewPokemon->getContentSize() + CONFIRM_DIALOGUE_SIZE_OFFSET, "Pokemon");
			node->autorelease();
			return node;
		}
		CC_SAFE_DELETE(node);
		return nullptr;
	}
	Popup * Popup::CreateShop()
	{
		Popup *node = new (std::nothrow)Popup();
		Size winSize = Director::getInstance()->getWinSize();
		if (node && node->init())
		{
			auto listViewItemShop = ui::ListView::create();
			listViewItemShop->setDirection(ui::ScrollView::Direction::HORIZONTAL);
			listViewItemShop->setAnchorPoint(Vec2(0.5, 0.5));
			listViewItemShop->setContentSize(Size(280, 140));
			listViewItemShop->setScale(0.8);
			listViewItemShop->setPosition(Vec2(winSize.width / 2, winSize.height / 2 - listViewItemShop->getContentSize().height / 4));
			listViewItemShop->setClippingEnabled(true);
			node->addChild(listViewItemShop, 200);
			auto listItem = Bag::GetInstance()->GetListItem();
			for (int i = 0; i < 10; i++)
			{
				ui::Button *buttonItem = ResourceManager::GetInstance()->GetButtonById(12);
				buttonItem->setTag(i);
				string id = to_string(listItem.at(i)->GetId());
				string name = listItem.at(i)->GetName();
				string price = "Price: " + to_string(listItem.at(i)->GetGold()) + " $";
				Sprite *sprite = ResourceManager::GetInstance()->GetSpriteById(137 + i);
				sprite->setTag(i);
				sprite->setScale(2.5);
				sprite->setPosition(buttonItem->getPosition().x + listViewItemShop->getContentSize().width* (i * 2 + 1) / 4,
					buttonItem->getPosition().y + listViewItemShop->getContentSize().height / 2);
				Label* labelName = ResourceManager::GetInstance()->GetLabelById(0);
				labelName->setString(name);
				labelName->setColor(Color3B(0, 0, 0));
				labelName->setPosition(buttonItem->getPosition().x + listViewItemShop->getContentSize().width* (i * 2 + 1) / 4,
					buttonItem->getPosition().y + listViewItemShop->getContentSize().height * 4 / 5);
				Label* labelNumber = ResourceManager::GetInstance()->GetLabelById(0);
				labelNumber->setString(price);
				labelNumber->setColor(Color3B(0, 0, 0));
				labelNumber->setPosition(buttonItem->getPosition().x + listViewItemShop->getContentSize().width* (i * 2 + 1) / 4,
					buttonItem->getPosition().y + listViewItemShop->getContentSize().height * 1 / 5);
				listViewItemShop->addChild(labelName, 202);
				listViewItemShop->addChild(labelNumber, 202);
				listViewItemShop->addChild(sprite, 202);
				listViewItemShop->pushBackCustomItem(buttonItem);
			}
			MenuItemImage *noButton = MenuItemImage::create(IMAGEPATH::CANCEL_BUTTON, IMAGEPATH::CANCEL_BUTTON_PRESSED, [node](Ref *sender) {
				node->dismiss(true);
				Buttons::GetIntance()->GetButtonBag()->setTouchEnabled(true);
			});
			Menu *menu = Menu::create(noButton, NULL);
			node->addChild(menu, 2);
			menu->setPosition(winSize.width / 2, winSize.height / 2 - listViewItemShop->getContentSize().height / 1.2);
			menu->alignItemsHorizontallyWithPadding(FONT::LABEL_OFFSET / 2);
			listViewItemShop->addEventListener((ListView::ccListViewCallback)CC_CALLBACK_2(Popup::SelectedItemEvent,node));
			CONFIRM_DIALOGUE_SIZE_OFFSET = Size(CONFIRM_DIALOGUE_SIZE_OFFSET.width, 150); 
			string str = "Shop item - Gold: " + to_string(Bag::GetInstance()->GetGold());
			node->initBg(listViewItemShop->getContentSize() + CONFIRM_DIALOGUE_SIZE_OFFSET, "Shop item");
			node->autorelease();
			return node;
		}
		CC_SAFE_DELETE(node);
		return nullptr;
	}
	void Popup::SelectedItemEvent(Ref * sender, ListView::EventType type)
	{
		ListView *listview = static_cast<ListView*>(sender);
		switch (type)
		{
		case cocos2d::ui::ListView::EventType::ON_SELECTED_ITEM_START:
			log("selected index start = %d", listview->getCurSelectedIndex());
			break;
		case cocos2d::ui::ListView::EventType::ON_SELECTED_ITEM_END:
			log("selected index end = %d", listview->getCurSelectedIndex());

			break;
		default:
			break;
		}
	}
	Popup *Popup::create(const std::string &title, const std::string &msg, cocos2d::Label *lbl, const std::function<void()> &YesFunc)
	{
		Popup *node = new (std::nothrow)Popup();
		Size winSize = Director::getInstance()->getWinSize();
		if (node && node->init())
		{

			if (!lbl) {
				lbl = Label::createWithTTF(msg, FONT::GAME_FONT, 20);
				lbl->setWidth(300);
			}
			lbl->setPosition(winSize.width / 2, winSize.height / 2 - FONT::LABEL_OFFSET / 2);
			lbl->enableOutline(Color4B::BLACK, FONT::LABEL_STROKE);
			lbl->setAlignment(cocos2d::TextHAlignment::CENTER, cocos2d::TextVAlignment::CENTER);
			lbl->enableShadow(Color4B::BLACK, Size(0, -2));
			if (YesFunc) {
				MenuItemImage *yesButton = MenuItemImage::create(IMAGEPATH::OK_BUTTON, IMAGEPATH::OK_BUTTON_PRESSED, [=](Ref *sender) {

					YesFunc();
					node->dismiss(true);
				});

				MenuItemImage *noButton = MenuItemImage::create(IMAGEPATH::CANCEL_BUTTON, IMAGEPATH::CANCEL_BUTTON_PRESSED, [node](Ref *sender) {
					node->dismiss(true);
				});


				Menu *menu = Menu::create(yesButton, noButton, NULL);
				node->addChild(menu, 2);
				menu->setPosition(winSize.width / 2, winSize.height / 2 - lbl->getContentSize().height / 2 - 75);
				menu->alignItemsHorizontallyWithPadding(FONT::LABEL_OFFSET / 2);

				lbl->setPosition(winSize / 2);
				CONFIRM_DIALOGUE_SIZE_OFFSET = Size(CONFIRM_DIALOGUE_SIZE_OFFSET.width, 250);
			}
			else
			{
				MenuItemImage *noButton = MenuItemImage::create(IMAGEPATH::CANCEL_BUTTON, IMAGEPATH::CANCEL_BUTTON_PRESSED, [node](Ref *sender) {
					node->dismiss(true);
				});
				Menu *menu = Menu::create(noButton, NULL);
				node->addChild(menu, 2);
				menu->setPosition(winSize.width / 2, winSize.height / 2 - lbl->getContentSize().height / 2 - 75);
				menu->alignItemsHorizontallyWithPadding(FONT::LABEL_OFFSET / 2);

				lbl->setPosition(winSize / 2);
				CONFIRM_DIALOGUE_SIZE_OFFSET = Size(CONFIRM_DIALOGUE_SIZE_OFFSET.width, 250);
			}
			node->addChild(lbl, 3);
			node->initBg(lbl->getContentSize() + CONFIRM_DIALOGUE_SIZE_OFFSET, title);
			node->autorelease();
			return node;
		}

		CC_SAFE_DELETE(node);
		return nullptr;
	}
	void Popup::initBg(Size size, const std::string &title)
	{
		Size winSize = Director::getInstance()->getWinSize();
		_bg = ui::ImageView::create(IMAGEPATH::BACKGROUND_IMAGE);
		this->addChild(_bg);

		_bg->setPosition(Point(winSize.width / 2, winSize.height / 2));
		_bg->setScale9Enabled(true);
		_bg->setContentSize(Size(size.width, size.height));

		ui::ImageView *fill = ui::ImageView::create(IMAGEPATH::BACKGROUND_IMAGE);
		_bg->addChild(fill);
		fill->setColor(Color3B(210, 210, 210));
		fill->setScale9Enabled(true);
		fill->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
		fill->setPosition(Point(FONT::LABEL_OFFSET / 4, FONT::LABEL_OFFSET / 4));
		fill->setContentSize(Size(size.width - FONT::LABEL_OFFSET / 2, size.height - FONT::LABEL_OFFSET * 2));


		Label *heading = Label::createWithTTF(title, FONT::GAME_FONT, FONT::TITLE_TEXT_SIZE);
		heading->setPosition(_bg->getContentSize().width / 2, _bg->getContentSize().height - FONT::LABEL_OFFSET);
		_bg->addChild(heading);
		heading->enableOutline(Color4B::BLACK, FONT::LABEL_STROKE);
		heading->enableShadow(Color4B::BLACK, Size(0, -3));
	}
}


