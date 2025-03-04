
#include "PokemonCenter.h"
#include "ResourceManager.h"
#include "SimpleAudioEngine.h"
#include "ResourceManager.h"
#include "Buttons.h"
#include "City.h"
#include "Model.h"
#include "Bag.h"
#include "Popup.h"

using namespace CocosDenshion;
USING_NS_CC;
Size pcVisibleSize;
Size pctileMapSize;

PhysicsBody* pcbody, *pcgateWay, *nursebody, *shopbody;
Camera *pccamera, *cameraUIPC;
Layer *layer_UI_PC;
int PokemonCenter::previousScene = 0;
Scene* PokemonCenter::createScene()
{
	auto scene = Scene::createWithPhysics();
	//scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);
	auto layer = PokemonCenter::create();
	scene->addChild(layer);
	pccamera = scene->getDefaultCamera();
	return scene;
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

// on "init" you need to initialize your instance
bool PokemonCenter::init()
{
	auto audio = SimpleAudioEngine::getInstance();
	audio->playBackgroundMusic("res/Sound/PokemonCenter.mp3", true);
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }

	pcVisibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
	
	auto map = ResourceManager::GetInstance()->GetTiledMapById(8);
	pctileMapSize = map->getContentSize();
	addChild(map);
	auto mPhysicsLayer = map->getLayer("physics");
	Size layerSize = mPhysicsLayer->getLayerSize();
	for (int i = 0; i < layerSize.width; i++)
	{
		for (int j = 0; j < layerSize.height; j++)
		{
			auto tileSet = mPhysicsLayer->getTileAt(Vec2(i, j));
			if (tileSet != NULL)
			{
				auto physics = PhysicsBody::createBox(tileSet->getContentSize(), PHYSICSBODY_MATERIAL_DEFAULT);
				physics->setCollisionBitmask(Model::BITMASK_WORLD);
				physics->setContactTestBitmask(true);
				physics->setDynamic(false);
				physics->setGravityEnable(false);
				tileSet->setPhysicsBody(physics);
			}
		}
	}

	InitObject();
	
	Button *up = Buttons::GetIntance()->GetButtonUp();
	Button *bag = Buttons::GetIntance()->GetButtonBag();
	Button *tips = Buttons::GetIntance()->GetButtonTips();

	layer_UI_PC = Layer::create();
	layer_UI_PC->setScale(pcVisibleSize.width / layer_UI_PC->getContentSize().width, pcVisibleSize.height / layer_UI_PC->getContentSize().height);
	up->setPosition(Vec2(pcVisibleSize.width / 7.2, pcVisibleSize.height / 3.6));
	bag->setPosition(Vec2(pcVisibleSize.width / 1.09, pcVisibleSize.height / 1.09));
	tips->setPosition(Vec2(pcVisibleSize.width / 20, pcVisibleSize.height / 1.09));
	cameraUIPC = Camera::create();
	cameraUIPC->setCameraMask(2);
	cameraUIPC->setCameraFlag(CameraFlag::USER1);
	up->setCameraMask(2);
	bag->setCameraMask(2);
	tips->setCameraMask(2);
	layer_UI_PC->addChild(cameraUIPC, 2);
	layer_UI_PC->addChild(up);
	layer_UI_PC->addChild(bag);
	layer_UI_PC->addChild(tips);
	this->addChild(layer_UI_PC, 100);

	Buttons::GetIntance()->GetButtonBag()->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type)
	{
		if (type == Widget::TouchEventType::ENDED)
		{
			Buttons::GetIntance()->GetButtonBag()->setTouchEnabled(false);
			string str = "My bag - Gold: " + to_string(Bag::GetInstance()->GetGold()) + " $";
			UICustom::Popup *popup = UICustom::Popup::createBag(str);
			popup->removeFromParent();
			popup->setAnchorPoint(Vec2(0.5, 0.5));
			popup->setPosition(pccamera->getPosition().x - popup->getContentSize().width / 2,
				pccamera->getPosition().y - popup->getContentSize().height / 2);
			this->addChild(popup, 101);
		}
	});

	Buttons::GetIntance()->GetButtonTips()->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type)
	{
		if (type == Widget::TouchEventType::ENDED)
		{
			Buttons::GetIntance()->GetButtonTips()->setTouchEnabled(false);
			UICustom::Popup *popup = UICustom::Popup::createAsMessage("Doctor", Model::GetTipsGame());
			popup->removeFromParent();
			popup->setAnchorPoint(Vec2(0.5, 0.5));
			popup->setPosition(pccamera->getPosition().x - popup->getContentSize().width / 2,
				pccamera->getPosition().y - popup->getContentSize().height / 2);
			this->addChild(popup, 101);
		}
	});

	Buttons::GetIntance()->ButtonListener(this->mPlayer);

	auto contactListener = EventListenerPhysicsContact::create();
	contactListener->onContactBegin = CC_CALLBACK_1(PokemonCenter::onContactBegin, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(contactListener, this);

	scheduleUpdate();
	this->m_messageBox = ResourceManager::GetInstance()->GetSpriteById(130);
	auto scale_x = 0.7;
	auto scale_y = 0.7;
	this->m_messageBox->setScaleX(scale_x);
	this->m_messageBox->setScaleY(scale_y);
	this->m_messageBox->setVisible(false);
	this->m_messageBox->setPosition(Director::getInstance()->getVisibleSize().width / 1.88, Director::getInstance()->getVisibleSize().height / 4);
	this->addChild(this->m_messageBox, 0);
	this->m_labelLog = ResourceManager::GetInstance()->GetLabelById(0);
	this->m_labelLog->setAnchorPoint(Vec2::ZERO);
	this->m_labelLog->setScale(1.5);
	this->m_labelLog->setTextColor(Color4B::BLACK);
	this->m_labelLog->setPosition(this->m_messageBox->getContentSize().width * scale_x / 10, this->m_messageBox->getContentSize().height * scale_y / 1.2);
	this->m_messageBox->addChild(this->m_labelLog);
	return true;
}

void PokemonCenter::TypeWriter(float deltaTime)
{
	if (writing < this->m_labelLog->getStringLength())
	{
		auto letter = this->m_labelLog->getLetter(writing);
		if (letter != nullptr)
		{
			letter->setOpacity(255);
		}
		writing++;
	}
	else
	{
		writing = 0;
		this->m_labelLog->setOpacity(255);
		this->unschedule(schedule_selector(PokemonCenter::TypeWriter));
	}
}

void PokemonCenter::LogSetOpacity(GLubyte opacity)
{
	for (int i = 0; i < this->m_labelLog->getStringLength(); i++)
	{
		auto letter = this->m_labelLog->getLetter(i);
		if (letter != nullptr)
		{
			letter->setOpacity(opacity);
		}
	}
}

bool PokemonCenter::onContactBegin(PhysicsContact & contact)
{
	PhysicsBody* a = contact.getShapeA()->getBody();
	PhysicsBody* b = contact.getShapeB()->getBody();

	if ((a->getCollisionBitmask() == Model::BITMASK_PLAYER && b->getCollisionBitmask() == Model::BITMASK_GATEWAY_TO_CITY)
		|| (a->getCollisionBitmask() == Model::BITMASK_GATEWAY_TO_CITY && b->getCollisionBitmask() == Model::BITMASK_PLAYER))
	{
		Buttons::GetIntance()->Remove();
		Director::getInstance()->getRunningScene()->pause();
		Director::getInstance()->replaceScene(TransitionFade::create(1.0f, City::createScene()));
		auto audio = SimpleAudioEngine::getInstance();
		audio->playEffect("res/Sound/ExitRoom.mp3", false);
	}
	else if ((a->getCollisionBitmask() == Model::BITMASK_WORLD && b->getCollisionBitmask() == Model::BITMASK_PLAYER)
		|| (a->getCollisionBitmask() == Model::BITMASK_PLAYER && b->getCollisionBitmask() == Model::BITMASK_WORLD))
	{
		auto audio = SimpleAudioEngine::getInstance();
		audio->playEffect("res/Sound/WallBump.mp3", false);
		switch (Buttons::state)
		{
		case 1:
			mPlayer->GetSpriteFront()->stopActionByTag(0);
			mPlayer->GetSpriteFront()->setPositionY(mPlayer->GetSpriteFront()->getPositionY() - 2);
			break;
		case 2:
			mPlayer->GetSpriteFront()->stopActionByTag(6);
			mPlayer->GetSpriteFront()->setPositionX(mPlayer->GetSpriteFront()->getPositionX() - 2);
			break;
		case 3:
			mPlayer->GetSpriteFront()->stopActionByTag(4);
			mPlayer->GetSpriteFront()->setPositionX(mPlayer->GetSpriteFront()->getPositionX() + 2);
			break;
		case 4:
			mPlayer->GetSpriteFront()->stopActionByTag(2);
			mPlayer->GetSpriteFront()->setPositionY(mPlayer->GetSpriteFront()->getPositionY() + 2);
			break;
		default:
			break;
		}
	}
	else if ((a->getCollisionBitmask() == Model::BITMASK_PLAYER && b->getCollisionBitmask() == Model::BITMASK_NURSENPC)
		|| a->getCollisionBitmask() == Model::BITMASK_NURSENPC && b->getCollisionBitmask() == Model::BITMASK_PLAYER)
	{
		switch (Buttons::state)
		{
		case 1:
			mPlayer->StopWalkUp();
			mPlayer->GetSpriteFront()->setPositionY(mPlayer->GetSpriteFront()->getPositionY() - 2);
			break;
		case 2:
			mPlayer->StopWalkRight();
			mPlayer->GetSpriteFront()->setPositionX(mPlayer->GetSpriteFront()->getPositionX() - 2);
			break;
		case 3:
			mPlayer->StopWalkLeft();
			mPlayer->GetSpriteFront()->setPositionX(mPlayer->GetSpriteFront()->getPositionX() + 2);
			break;
		case 4:
			mPlayer->StopWalkDown();
			mPlayer->GetSpriteFront()->setPositionY(mPlayer->GetSpriteFront()->getPositionY() + 2);
			break;
		default:
			break;
		}
		auto audio = SimpleAudioEngine::getInstance();
		audio->playEffect("res/Sound/recovery.wav", false);
		Buttons::GetIntance()->SetVisible(false);
		this->Log("We've restored your pokemon to full health.");
		this->m_messageBox->setVisible(true);
		touchListener = EventListenerTouchOneByOne::create();
		touchListener->onTouchBegan = CC_CALLBACK_2(PokemonCenter::onTouchBegan, this);
		_eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);
		
		auto ListYP = Bag::GetInstance()->GetListPokemon();
		for (int i = 0; i < ListYP.size(); i++)
		{
			if (ListYP.at(i) != nullptr)
			{
				ListYP.at(i)->Restore();
			}
		}
	}
	else if ((a->getCollisionBitmask() == Model::BITMASK_PLAYER && b->getCollisionBitmask() == Model::BITMASK_SHOPNPC)
		|| a->getCollisionBitmask() == Model::BITMASK_SHOPNPC && b->getCollisionBitmask() == Model::BITMASK_PLAYER)
	{
		switch (Buttons::state)
		{
		case 1:
			mPlayer->StopWalkUp();
			mPlayer->GetSpriteFront()->setPositionY(mPlayer->GetSpriteFront()->getPositionY() - 2);
			break;
		case 2:
			mPlayer->StopWalkRight();
			mPlayer->GetSpriteFront()->setPositionX(mPlayer->GetSpriteFront()->getPositionX() - 2);
			break;
		case 3:
			mPlayer->StopWalkLeft();
			mPlayer->GetSpriteFront()->setPositionX(mPlayer->GetSpriteFront()->getPositionX() + 2);
			break;
		case 4:
			mPlayer->StopWalkDown();
			mPlayer->GetSpriteFront()->setPositionY(mPlayer->GetSpriteFront()->getPositionY() + 2);
			break;
		default:
			break;
		}
		auto audio = SimpleAudioEngine::getInstance();
		audio->playEffect("res/Sound/Beep.mp3", false);
		touchListener = EventListenerTouchOneByOne::create();
		touchListener->onTouchBegan = CC_CALLBACK_2(PokemonCenter::onTouchBegan, this);
		_eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);
		UICustom::Popup *popupShop = UICustom::Popup::CreateShop();
		this->addChild(popupShop);
	}
	return true;

}

void PokemonCenter::InitObject()
{
	auto map = ResourceManager::GetInstance()->GetTiledMapById(8);
	auto m_objectGroup = map->getObjectGroup("Object");
	auto objects = m_objectGroup->getObjects();
	for (int i = 0; i < objects.size(); i++) {
		auto object = objects.at(i);
		auto properties = object.asValueMap();
		float posX = properties.at("x").asFloat();
		float posY = properties.at("y").asFloat();
		int type = object.asValueMap().at("type").asInt();
		if (type == Model::MODLE_TYPE_MAIN_CHARACTER) {
			int preScene = object.asValueMap().at("pre").asInt();
			if (preScene == previousScene) {
				mPlayer = new Trainer(this);
				mPlayer->GetSpriteFront()->setPosition(Vec2(posX, posY));
				pcbody = PhysicsBody::createBox(mPlayer->GetSpriteFront()->getContentSize(), PHYSICSBODY_MATERIAL_DEFAULT);
				pcbody->setCollisionBitmask(Model::BITMASK_PLAYER);
				pcbody->setContactTestBitmask(true);
				pcbody->setDynamic(true);
				pcbody->setRotationEnable(false);
				pcbody->setGravityEnable(false);
				mPlayer->GetSpriteFront()->setPhysicsBody(pcbody);
			}
			else continue;
		}
		else if (type== Model::MODLE_TYPE_NURSENPC)
		{
			m_nurse = ResourceManager::GetInstance()->GetSpriteById(126);
			m_nurse->setPosition(Vec2(posX, posY));
			m_nurse->setScale(0.8);
			nursebody = PhysicsBody::createBox(m_nurse->getContentSize(), PHYSICSBODY_MATERIAL_DEFAULT);
			nursebody->setCollisionBitmask(Model::BITMASK_NURSENPC);
			nursebody->setContactTestBitmask(true);
			nursebody->setDynamic(false);
			nursebody->setGravityEnable(false);
			m_nurse->setPhysicsBody(nursebody);
			this->addChild(m_nurse, 10);
		}
		else if (type == Model::MODLE_TYPE_SHOPNPC)
		{
			m_shop = ResourceManager::GetInstance()->GetSpriteById(127);
			m_shop->setPosition(Vec2(posX, posY));
			m_shop->setScale(0.8);
			shopbody = PhysicsBody::createBox(m_shop->getContentSize(), PHYSICSBODY_MATERIAL_DEFAULT);
			shopbody->setCollisionBitmask(Model::BITMASK_SHOPNPC);
			shopbody->setContactTestBitmask(true);
			shopbody->setDynamic(false);
			shopbody->setGravityEnable(false);
			m_shop->setPhysicsBody(shopbody);
			this->addChild(m_shop, 10);
		}
		else {
			mGateWay = Sprite::create("res/walkup.png");
			mGateWay->setPosition(Vec2(posX, posY));
			pcgateWay = PhysicsBody::createBox(mGateWay->getContentSize(), PHYSICSBODY_MATERIAL_DEFAULT);
			pcgateWay->setCollisionBitmask(Model::BITMASK_GATEWAY_TO_CITY);
			pcgateWay->setContactTestBitmask(true);
			pcgateWay->setDynamic(false);
			pcgateWay->setGravityEnable(false);
			mGateWay->setPhysicsBody(pcgateWay);
			mGateWay->setVisible(false);
			this->addChild(mGateWay, 10);
		}
	}
}

void PokemonCenter::UpdateCamera() {
	if (pcVisibleSize.width >= pctileMapSize.width) {
		if (pcVisibleSize.height >= pctileMapSize.height) {
			pccamera->setPosition(pctileMapSize / 2);
		}
		else
		{
			if (abs(mPlayer->GetSpriteFront()->getPosition().y - pctileMapSize.height / 2)>abs(pctileMapSize.height / 2 - pcVisibleSize.height / 2)) {
				pccamera->setPosition(pctileMapSize.width / 2, (mPlayer->GetSpriteFront()->getPosition().y >pccamera->getPosition().y) ? (pctileMapSize.height - pcVisibleSize.height / 2) : pcVisibleSize.height / 2);
			}
			else {
				pccamera->setPosition(pctileMapSize.width / 2, mPlayer->GetSpriteFront()->getPosition().y);
			}
		}
	}
	else {
		if (pcVisibleSize.height >= pctileMapSize.height) {
			if (abs(mPlayer->GetSpriteFront()->getPosition().x - pctileMapSize.width / 2)>abs(pctileMapSize.width / 2 - pcVisibleSize.width / 2)) {
				pccamera->setPosition((mPlayer->GetSpriteFront()->getPosition().y >pccamera->getPosition().y) ? (pctileMapSize.width - pcVisibleSize.width / 2) : pcVisibleSize.width / 2, pctileMapSize.height / 2);
			}
			else {
				pccamera->setPosition(mPlayer->GetSpriteFront()->getPosition().x, pctileMapSize.height / 2);
			}
		}
		else {
			if (abs(mPlayer->GetSpriteFront()->getPosition().x - pctileMapSize.width / 2)>abs(pctileMapSize.width / 2 - pcVisibleSize.width / 2)
				&& abs(mPlayer->GetSpriteFront()->getPosition().y - pctileMapSize.height / 2)>abs(pctileMapSize.height / 2 - pcVisibleSize.height / 2)) {
				pccamera->setPosition((mPlayer->GetSpriteFront()->getPosition().x >pccamera->getPosition().x) ? (pctileMapSize.width - pcVisibleSize.width / 2) : pcVisibleSize.width / 2, (mPlayer->GetSpriteFront()->getPosition().y >pccamera->getPosition().y) ? (pctileMapSize.height - pcVisibleSize.height / 2) : pcVisibleSize.height / 2);
			}
			else if (abs(mPlayer->GetSpriteFront()->getPosition().x - pctileMapSize.width / 2)>abs(pctileMapSize.width / 2 - pcVisibleSize.width / 2)
				&& abs(mPlayer->GetSpriteFront()->getPosition().y - pctileMapSize.height / 2)<abs(pctileMapSize.height / 2 - pcVisibleSize.height / 2)) {
				pccamera->setPosition((mPlayer->GetSpriteFront()->getPosition().x >pccamera->getPosition().x) ? (pctileMapSize.width - pcVisibleSize.width / 2) : pcVisibleSize.width / 2, mPlayer->GetSpriteFront()->getPosition().y);
			}
			else if (abs(mPlayer->GetSpriteFront()->getPosition().x - pctileMapSize.width / 2)<abs(pctileMapSize.width / 2 - pcVisibleSize.width / 2)
				&& abs(mPlayer->GetSpriteFront()->getPosition().y - pctileMapSize.height / 2)>abs(pctileMapSize.height / 2 - pcVisibleSize.height / 2)) {
				pccamera->setPosition(mPlayer->GetSpriteFront()->getPosition().x, (mPlayer->GetSpriteFront()->getPosition().y >pccamera->getPosition().y) ? (pctileMapSize.height - pcVisibleSize.height / 2) : pcVisibleSize.height / 2);
			}
			else {
				pccamera->setPosition(mPlayer->GetSpriteFront()->getPosition());
			}
		}
	}
}
void PokemonCenter::Log(string logg)
{
	this->m_labelLog->setString(logg);
	this->LogSetOpacity(0);
	this->m_labelLog->setOpacity(0);
	writing = 0;
	this->schedule(schedule_selector(PokemonCenter::TypeWriter), 0.05);
}
bool PokemonCenter::onTouchBegan(Touch * touch, Event * e)
{
	if (this->m_labelLog->getOpacity() == 0)
	{
		this->unschedule(schedule_selector(PokemonCenter::TypeWriter));
		this->LogSetOpacity(255);
		this->m_labelLog->setOpacity(255);
		/*auto touchListener = EventListenerTouchOneByOne::create();
		touchListener->onTouchBegan = CC_CALLBACK_2(PokemonCenter::onTouchEnd, this);
		_eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);*/
	}
	else
	{
		Buttons::GetIntance()->SetVisible(true);
		this->m_messageBox->setVisible(false);
		Director::getInstance()->getEventDispatcher()->removeEventListener(touchListener);
	}
	return true;
}

bool PokemonCenter::onTouchEnd(Touch * t, Event * event)
{
	/*this->m_messageBox->setVisible(false);
	Buttons::GetIntance()->SetTouchEnable();*/
	return true;
}

int pcSum = 0;

void PokemonCenter::UpdatePlayer(float dt) {
	pcSum++;
	if (pcSum >30) {
		if (mPlayer->isMoveDown) {
			mPlayer->StopWalkDown();
			mPlayer->WalkDown();
		}
		else if (mPlayer->isMoveLeft) {
			mPlayer->StopWalkLeft();
			mPlayer->WalkLeft();
		}
		else if (mPlayer->isMoveUp) {
			mPlayer->StopWalkUp();
			mPlayer->WalkUp();
		}
		else if (mPlayer->isMoveRight) {
			mPlayer->StopWalkRight();
			mPlayer->WalkRight();
		}
		else
		{
		}
		pcSum = 0;
	}
}


void PokemonCenter::update(float dt)
{
	UpdatePlayer(dt);
	UpdateCamera();
}