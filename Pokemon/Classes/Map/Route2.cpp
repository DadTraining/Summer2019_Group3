
#include "Route2.h"
#include "ResourceManager.h"
#include "SimpleAudioEngine.h"
#include "Buttons.h"
#include "City.h"
#include "Model.h"
#include "Scene\BattleScene.h"
#include "VictoryRoad.h"
#include "Popup.h"

using namespace CocosDenshion;
Size route2VisibleSize;
Size route2TileMapSize;

vector<Vec2> route2_point;
float route2_tick = 0;

Layer *layer_UI_Route2;
Camera *route2Camera, *cameraUIRoute2;
PhysicsBody* route2Body, *route2GateWay, *raikoubody, *roadnpcbody;
int Route2::previousScene = 0;
bool route2_state = true;
vector<Pokemon*> listPokemon;

Scene* Route2::createScene()
{
	auto scene = Scene::createWithPhysics();
	//scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);
	auto layer = Route2::create();
	scene->addChild(layer);
	route2Camera = scene->getDefaultCamera();
	return scene;
}

void Route2::TypeWriter(float deltaTime)
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
		this->unschedule(schedule_selector(Route2::TypeWriter));
	}
}

void Route2::LogSetOpacity(GLubyte opacity)
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

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
	printf("Error while loading: %s\n", filename);
	printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

// on "init" you need to initialize your instance
bool Route2::init()
{
	auto audio = SimpleAudioEngine::getInstance();
	audio->playBackgroundMusic("res/Sound/Route2.mp3", true);
	//////////////////////////////
	// 1. super init first
	if (!Layer::init())
	{
		return false;
	}

	route2VisibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	map = ResourceManager::GetInstance()->GetTiledMapById(10);
	route2TileMapSize = map->getContentSize();
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

	InitGrass();

	InitObject();

	Button *up = Buttons::GetIntance()->GetButtonUp();
	Button *bag = Buttons::GetIntance()->GetButtonBag();
	Button *tips = Buttons::GetIntance()->GetButtonTips();

	layer_UI_Route2 = Layer::create();
	layer_UI_Route2->setScale(route2VisibleSize.width / layer_UI_Route2->getContentSize().width, route2VisibleSize.height / layer_UI_Route2->getContentSize().height);
	up->setPosition(Vec2(route2VisibleSize.width / 7.2, route2VisibleSize.height / 3.6));
	bag->setPosition(Vec2(route2VisibleSize.width / 1.09, route2VisibleSize.height / 1.09));
	tips->setPosition(Vec2(route2VisibleSize.width / 20, route2VisibleSize.height / 1.09));
	cameraUIRoute2 = Camera::create();
	cameraUIRoute2->setCameraMask(2);
	cameraUIRoute2->setCameraFlag(CameraFlag::USER1);
	up->setCameraMask(2);
	bag->setCameraMask(2);
	tips->setCameraMask(2);
	layer_UI_Route2->addChild(cameraUIRoute2, 2);
	layer_UI_Route2->addChild(up);
	layer_UI_Route2->addChild(bag);
	layer_UI_Route2->addChild(tips);
	this->addChild(layer_UI_Route2, 100);

	Buttons::GetIntance()->ButtonListener(this->mPlayer);

	Buttons::GetIntance()->GetButtonBag()->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type)
	{
		if (type == Widget::TouchEventType::ENDED)
		{
			Buttons::GetIntance()->GetButtonBag()->setTouchEnabled(false);
			string str = "My bag - Gold: " + to_string(Bag::GetInstance()->GetGold()) + " $";
			UICustom::Popup *popup = UICustom::Popup::createBag(str);
			popup->removeFromParent();
			popup->setAnchorPoint(Vec2(0.5, 0.5));
			popup->setPosition(route2Camera->getPosition().x - popup->getContentSize().width / 2,
				route2Camera->getPosition().y - popup->getContentSize().height / 2);
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
			popup->setPosition(route2Camera->getPosition().x - popup->getContentSize().width / 2,
				route2Camera->getPosition().y - popup->getContentSize().height / 2);
			this->addChild(popup, 101);
		}
	});
	auto contactListener = EventListenerPhysicsContact::create();
	contactListener->onContactBegin = CC_CALLBACK_1(Route2::onContactBegin, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(contactListener, this);

	scheduleUpdate();
	this->m_messageBox = ResourceManager::GetInstance()->GetSpriteById(130);
	auto scale_x = 0.7;
	auto scale_y = 0.7;
	this->m_messageBox->setScaleX(scale_x);
	this->m_messageBox->setScaleY(scale_y);
	this->m_messageBox->setVisible(false);
	this->m_messageBox->setPosition(Director::getInstance()->getVisibleSize().width / 1.76, Director::getInstance()->getVisibleSize().height / 1.5);
	this->addChild(this->m_messageBox, 10);
	this->m_labelLog = ResourceManager::GetInstance()->GetLabelById(0);
	this->m_labelLog->setAnchorPoint(Vec2::ZERO);
	this->m_labelLog->setScale(1.5);
	this->m_labelLog->setTextColor(Color4B::BLACK);
	this->m_labelLog->setPosition(this->m_messageBox->getContentSize().width * scale_x / 10, this->m_messageBox->getContentSize().height * scale_y / 1.2);
	this->m_messageBox->addChild(this->m_labelLog);
	return true;
}

bool Route2::onContactBegin(PhysicsContact& contact)

{
	PhysicsBody* a = contact.getShapeA()->getBody();
	PhysicsBody* b = contact.getShapeB()->getBody();

	if ((a->getCollisionBitmask() == Model::BITMASK_PLAYER && b->getCollisionBitmask() == Model::BITMASK_ROUTE2_GATE_TO_CITY)
		|| a->getCollisionBitmask() == Model::MODLE_TYPE_ROUTE2_GATE_TO_CITY && b->getCollisionBitmask() == Model::BITMASK_PLAYER)
	{
		Buttons::GetIntance()->Remove();
		Director::getInstance()->getRunningScene()->pause();
		Route2::previousScene = Model::PRESCENE_CITY_TO_ROUTE2;
		Director::getInstance()->replaceScene(TransitionFade::create(1.0f, City::createScene()));
		auto audio = SimpleAudioEngine::getInstance();
		audio->playEffect("res/Sound/ExitRoom.mp3", false);
	}
	else if ((a->getCollisionBitmask() == Model::BITMASK_PLAYER && b->getCollisionBitmask() == Model::BITMASK_ROUTE2_GATE_TO_ROAD)
		|| a->getCollisionBitmask() == Model::BITMASK_ROUTE2_GATE_TO_ROAD && b->getCollisionBitmask() == Model::BITMASK_PLAYER)
	{
		Buttons::GetIntance()->Remove();
		Route2::previousScene = Model::PRESCENE_ROAD_TO_ROUTE2;
		Director::getInstance()->getRunningScene()->pause();
		Director::getInstance()->replaceScene(TransitionFade::create(1.0f, Road::createScene()));
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
	else if ((a->getCollisionBitmask() == Model::BITMASK_PLAYER && b->getCollisionBitmask() == Model::BITMASK_RAIKOU)
		|| a->getCollisionBitmask() == Model::BITMASK_RAIKOU && b->getCollisionBitmask() == Model::BITMASK_PLAYER)
	{
		switch (Buttons::state)
		{
		case 1:
			mPlayer->StopWalkUp();
			break;
		case 2:
			mPlayer->StopWalkRight();
			break;
		case 3:
			mPlayer->StopWalkLeft();
			break;
		case 4:
			mPlayer->StopWalkDown();
			break;
		default:
			break;
		}
		auto audio = SimpleAudioEngine::getInstance();
		audio->playEffect("res/Sound/Beep.mp3", false);
		Buttons::GetIntance()->SetVisible(false);
		this->Log("Rararaiiiii!");
		this->m_messageBox->setVisible(true);
		touchListener = EventListenerTouchOneByOne::create();
		touchListener->onTouchBegan = CC_CALLBACK_2(Route2::onTouchBegan, this);
		_eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);
		listPokemon = { new Raikou(15) };
		auto listener = CallFunc::create([this]() {
			if (route2_state == false)
			{
				Model::RAIKOU = false;
				this->m_raikou->removeFromParent();
				route2_state = true;
				this->stopActionByTag(100);
			}
		});
		auto rp = RepeatForever::create(Spawn::create(listener, nullptr));
		rp->setTag(100);
		this->runAction(rp);
		//removeChild(m_raikou, true);
	}
	else if ((a->getCollisionBitmask() == Model::BITMASK_PLAYER && b->getCollisionBitmask() == Model::BITMASK_ROADNPC && Bag::GetInstance()->GetCountPokemon() > 0)
		|| a->getCollisionBitmask() == Model::BITMASK_ROADNPC && b->getCollisionBitmask() == Model::BITMASK_PLAYER && Bag::GetInstance()->GetCountPokemon() > 0)
	{
		switch (Buttons::state)
		{
		case 1:
			mPlayer->StopWalkUp();
			break;
		case 2:
			mPlayer->StopWalkRight();
			break;
		case 3:
			mPlayer->StopWalkLeft();
			break;
		case 4:
			mPlayer->StopWalkDown();
			break;
		default:
			break;
		}
		auto audio = SimpleAudioEngine::getInstance();
		audio->playEffect("res/Sound/Beep.mp3", false);
		Buttons::GetIntance()->SetVisible(false);
		this->Log("Let's battle!");
		this->m_messageBox->setVisible(true);
		touchListener = EventListenerTouchOneByOne::create();
		touchListener->onTouchBegan = CC_CALLBACK_2(Route2::onTouchBegan, this);
		_eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);
		listPokemon = this->m_roadnpc->GetListPokemon();
		auto listener = CallFunc::create([this]() {
			if (route2_state == false)
			{
				Model::ROADNPC = false;
				this->m_roadnpc->GetSpriteFront()->removeFromParent();
				route2_state = true;
				this->stopActionByTag(100);
			}
		});
		auto rp = RepeatForever::create(Spawn::create(listener, nullptr));
		rp->setTag(100);
		this->runAction(rp);
		//removeChild(m_roadnpc, true);
	}
	return true;

}

void Route2::InitGrass()
{
	auto grass = map->getLayer("grass");
	int width = grass->getLayerSize().width;
	int height = grass->getLayerSize().height;
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			auto tilePokemon = grass->getTileAt(Vec2(i, j));
			if (tilePokemon != NULL)
			{
				route2_point.push_back(tilePokemon->getPosition());
			}
		}
	}
}

void Route2::InitObject()
{
	auto map = ResourceManager::GetInstance()->GetTiledMapById(10);
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
				if (preScene == 1)
				{
					mPlayer->GetSpriteFront()->setTexture("res/Trainer/walkdown/1.png");
				}
				mPlayer->GetSpriteFront()->setPosition(Vec2(posX, posY));
				route2Body = PhysicsBody::createBox(mPlayer->GetSpriteFront()->getContentSize(), PHYSICSBODY_MATERIAL_DEFAULT);
				route2Body->setCollisionBitmask(Model::BITMASK_PLAYER);
				route2Body->setContactTestBitmask(true);
				route2Body->setDynamic(true);
				route2Body->setRotationEnable(false);
				route2Body->setGravityEnable(false);
				mPlayer->GetSpriteFront()->setPhysicsBody(route2Body);
			}
			else continue;
		}
		else if (type == Model::MODLE_TYPE_ROUTE2_GATE_TO_CITY)
		{
			mGateWay = Sprite::create("res/walkup.png");
			mGateWay->setPosition(Vec2(posX, posY));
			route2GateWay = PhysicsBody::createBox(mGateWay->getContentSize(), PHYSICSBODY_MATERIAL_DEFAULT);
			route2GateWay->setCollisionBitmask(Model::BITMASK_ROUTE2_GATE_TO_CITY);
			route2GateWay->setContactTestBitmask(true);
			route2GateWay->setDynamic(false);
			route2GateWay->setGravityEnable(false);
			mGateWay->setPhysicsBody(route2GateWay);
			mGateWay->setVisible(false);
			this->addChild(mGateWay, 10);
		}
		else if (type == Model::MODLE_TYPE_ROUTE2_GATE_TO_ROAD)
		{
			mGateWay = Sprite::create("res/walkup.png");
			mGateWay->setPosition(Vec2(posX, posY));
			route2GateWay = PhysicsBody::createBox(mGateWay->getContentSize(), PHYSICSBODY_MATERIAL_DEFAULT);
			route2GateWay->setCollisionBitmask(Model::BITMASK_ROUTE2_GATE_TO_ROAD);
			route2GateWay->setContactTestBitmask(true);
			route2GateWay->setDynamic(false);
			route2GateWay->setGravityEnable(false);
			mGateWay->setPhysicsBody(route2GateWay);
			mGateWay->setVisible(false);
			this->addChild(mGateWay, 10);
		}
		else if (type == Model::MODLE_TYPE_RAIKOU)
		{
			m_raikou = ResourceManager::GetInstance()->GetSpriteById(151);
			if (Model::RAIKOU == true)
			{
				m_raikou->setPosition(Vec2(posX, posY));
				m_raikou->setScale(2);
				raikoubody = PhysicsBody::createBox(m_raikou->getContentSize(), PHYSICSBODY_MATERIAL_DEFAULT);
				raikoubody->setCollisionBitmask(Model::BITMASK_RAIKOU);
				raikoubody->setContactTestBitmask(true);
				raikoubody->setDynamic(false);
				raikoubody->setGravityEnable(false);
				m_raikou->setPhysicsBody(raikoubody);
				this->addChild(m_raikou, 10);
			}
			else
			{
			}
		}
		else if (type == Model::MODLE_TYPE_ROADNPC)
		{
			m_roadnpc = new RoadNPC();
			if (Model::ROADNPC == true)
			{
				m_roadnpc->GetSpriteFront()->setPosition(Vec2(posX, posY));
				roadnpcbody = PhysicsBody::createBox(m_roadnpc->GetSpriteFront()->getContentSize(), PHYSICSBODY_MATERIAL_DEFAULT);
				roadnpcbody->setCollisionBitmask(Model::BITMASK_ROADNPC);
				roadnpcbody->setContactTestBitmask(true);
				roadnpcbody->setDynamic(false);
				roadnpcbody->setGravityEnable(false);
				m_roadnpc->GetSpriteFront()->setPhysicsBody(roadnpcbody);
				this->addChild(m_roadnpc->GetSpriteFront(), 10);
			}
			else
			{
			}
		}
	}
}

void Route2::UpdateCamera() {
	if (route2VisibleSize.width >= route2TileMapSize.width) {
		if (route2VisibleSize.height >= route2TileMapSize.height) {
			route2Camera->setPosition(route2TileMapSize / 2);
		}
		else
		{
			if (abs(mPlayer->GetSpriteFront()->getPosition().y - route2TileMapSize.height / 2)>abs(route2TileMapSize.height / 2 - route2VisibleSize.height / 2)) {
				route2Camera->setPosition(route2TileMapSize.width / 2, (mPlayer->GetSpriteFront()->getPosition().y >route2Camera->getPosition().y) ? (route2TileMapSize.height - route2VisibleSize.height / 2) : route2VisibleSize.height / 2);
			}
			else {
				route2Camera->setPosition(route2TileMapSize.width / 2, mPlayer->GetSpriteFront()->getPosition().y);
			}
		}
	}
	else {
		if (route2VisibleSize.height >= route2TileMapSize.height) {
			if (abs(mPlayer->GetSpriteFront()->getPosition().x - route2TileMapSize.width / 2)>abs(route2TileMapSize.width / 2 - route2VisibleSize.width / 2)) {
				route2Camera->setPosition((mPlayer->GetSpriteFront()->getPosition().y >route2Camera->getPosition().y) ? (route2TileMapSize.width - route2VisibleSize.width / 2) : route2VisibleSize.width / 2, route2TileMapSize.height / 2);
			}
			else {
				route2Camera->setPosition(mPlayer->GetSpriteFront()->getPosition().x, route2TileMapSize.height / 2);
			}
		}
		else {
			if (abs(mPlayer->GetSpriteFront()->getPosition().x - route2TileMapSize.width / 2)>abs(route2TileMapSize.width / 2 - route2VisibleSize.width / 2)
				&& abs(mPlayer->GetSpriteFront()->getPosition().y - route2TileMapSize.height / 2)>abs(route2TileMapSize.height / 2 - route2VisibleSize.height / 2)) {
				route2Camera->setPosition((mPlayer->GetSpriteFront()->getPosition().x >route2Camera->getPosition().x) ? (route2TileMapSize.width - route2VisibleSize.width / 2) : route2VisibleSize.width / 2, (mPlayer->GetSpriteFront()->getPosition().y >route2Camera->getPosition().y) ? (route2TileMapSize.height - route2VisibleSize.height / 2) : route2VisibleSize.height / 2);
			}
			else if (abs(mPlayer->GetSpriteFront()->getPosition().x - route2TileMapSize.width / 2)>abs(route2TileMapSize.width / 2 - route2VisibleSize.width / 2)
				&& abs(mPlayer->GetSpriteFront()->getPosition().y - route2TileMapSize.height / 2)<abs(route2TileMapSize.height / 2 - route2VisibleSize.height / 2)) {
				route2Camera->setPosition((mPlayer->GetSpriteFront()->getPosition().x >route2Camera->getPosition().x) ? (route2TileMapSize.width - route2VisibleSize.width / 2) : route2VisibleSize.width / 2, mPlayer->GetSpriteFront()->getPosition().y);
			}
			else if (abs(mPlayer->GetSpriteFront()->getPosition().x - route2TileMapSize.width / 2)<abs(route2TileMapSize.width / 2 - route2VisibleSize.width / 2)
				&& abs(mPlayer->GetSpriteFront()->getPosition().y - route2TileMapSize.height / 2)>abs(route2TileMapSize.height / 2 - route2VisibleSize.height / 2)) {
				route2Camera->setPosition(mPlayer->GetSpriteFront()->getPosition().x, (mPlayer->GetSpriteFront()->getPosition().y >route2Camera->getPosition().y) ? (route2TileMapSize.height - route2VisibleSize.height / 2) : route2VisibleSize.height / 2);
			}
			else {
				route2Camera->setPosition(mPlayer->GetSpriteFront()->getPosition());
			}
		}
	}
}

void Route2::update(float dt)
{
	if (this->getTag() == 10)
	{
		route2_state = false;
	}
	UpdatePlayer(dt);
	for (int i = 0; i < route2_point.size(); i++)
	{
		if (this->mPlayer->GetSpriteFront()->getBoundingBox().containsPoint(route2_point.at(i)) && Buttons::state != 0 && Bag::GetInstance()->GetCountPokemon() > 0)
		{
			route2_tick += dt;
			break;
		}
	}
	if (route2_tick >= 2.5)
	{
		vector<Pokemon*> wildPokemon;
		int index = rand() % 5 + 1;
		int level = rand() % 4 + 11;
		switch (index)
		{
		case 1:
			wildPokemon.push_back(new Swellow(level));
			break;
		case 2:
			wildPokemon.push_back(new Skymin(level));
			break;
		case 3:
			wildPokemon.push_back(new Greninja(level));
			break;
		case 4:
			wildPokemon.push_back(new Kirlia(level));
			break;
		case 5:
			wildPokemon.push_back(new Pikachu(level));
			break;
		default:
			break;
		}
		Buttons::GetIntance()->SetVisible(false);
		auto layer = BattleScene::CreateLayer(wildPokemon);
		layer->setPosition(route2Camera->getPosition().x - Director::getInstance()->getVisibleSize().width / 2,
			route2Camera->getPosition().y - Director::getInstance()->getVisibleSize().height / 2);
		this->addChild(layer, 1000);
		this->unscheduleUpdate();
		route2_tick = 0;
	}
	UpdateCamera();
	this->setTag(0);
}

int route2Sum = 0;

void Route2::UpdatePlayer(float dt) {
	route2Sum++;
	if (route2Sum >30) {
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
		route2Sum = 0;
	}
}

void Route2::Log(string logg)
{
	auto audio = SimpleAudioEngine::getInstance();
	audio->playEffect("Beep.mp3", false);
	this->m_labelLog->setString(logg);
	this->LogSetOpacity(0);
	this->m_labelLog->setOpacity(0);
	writing = 0;
	this->schedule(schedule_selector(Route2::TypeWriter), 0.05);
}

bool Route2::onTouchBegan(Touch * touch, Event * e)
{
	if (this->m_labelLog->getOpacity() == 0)
	{
		this->unschedule(schedule_selector(Route2::TypeWriter));
		this->LogSetOpacity(255);
		this->m_labelLog->setOpacity(255);
		/*auto touchListener = EventListenerTouchOneByOne::create();
		touchListener->onTouchBegan = CC_CALLBACK_2(Route2::onTouchEnd, this);
		_eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);*/
	}
	else
	{
		this->m_messageBox->setVisible(false);
		auto layer = BattleScene::CreateLayer(listPokemon);
		layer->setPosition(route2Camera->getPosition().x - Director::getInstance()->getVisibleSize().width / 2,
			route2Camera->getPosition().y - Director::getInstance()->getVisibleSize().height / 2);
		this->addChild(layer, 1000);
		this->unscheduleUpdate();
		//Buttons::GetIntance()->SetVisible(false);
		Director::getInstance()->getEventDispatcher()->removeEventListener(touchListener);
	}
	return true;
}

bool Route2::onTouchEnd(Touch * t, Event * event)
{
	/*this->m_messageBox->setVisible(false);
	Buttons::GetIntance()->SetTouchEnable();*/
	return true;
}