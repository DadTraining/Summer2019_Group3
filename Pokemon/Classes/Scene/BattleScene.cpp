#include "BattleScene.h"
#include "Popup.h"
#include "Buttons.h"
#include "Map\Town.h"
#include "Map\PokemonCenter.h"
#define scale_hpBar 0.47

BattleScene::BattleScene()
{
}

BattleScene::~BattleScene()
{
}

cocos2d::Scene * BattleScene::createScene()
{
	auto scene = Scene::create();
	auto layer = BattleScene::create();
	scene->addChild(layer);
	return scene;
}

bool BattleScene::init()
{
	auto audio = SimpleAudioEngine::getInstance();
	audio->playBackgroundMusic("res/Sound/BattleScene.mp3", true);
	if (!Layer::init())
	{
		return false;
	}

	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	this->InitTiledMap();

	this->InitUI();

	this->InitObject();

	this->AddEventListener();

	this->StartBattle();

	return true;
}

void BattleScene::update(float deltaTime)
{
}

vector<Pokemon*> listOpponentPokemon;
int countOppPokemons = 0;

Layer * BattleScene::CreateLayer(vector<Pokemon*> pokemons)
{
	listOpponentPokemon = pokemons;
	countOppPokemons = listOpponentPokemon.size();
	auto layer = BattleScene::create();
	return layer;
}

void BattleScene::ReleaseChildren()
{
	this->m_tiledmap->removeFromParent();
	this->m_player->RemoveFromParent();
	this->m_opponent->RemoveFromParent();
	auto listNode = this->getChildren();
	auto size = this->getChildrenCount();
	for (int i = 0; i < size; i++)
	{
		auto node = listNode.at(i);
		auto children = node->getChildren();
		auto _size = node->getChildrenCount();
		for (int j = 0; j < _size; j++)
		{
			auto child = children.at(j);
			child->removeFromParent();
			child->release();
		}
		node->removeFromParent();
		node->release();
	}
}

void BattleScene::TypeWriter(float deltaTime)
{
	if (writing < this->m_labelBattleLog->getStringLength())
	{
		auto letter = this->m_labelBattleLog->getLetter(writing);
		if (letter != nullptr)
		{
			letter->setOpacity(255);
		}
		writing++;
	}
	else
	{
		writing = 0;
		this->m_labelBattleLog->setOpacity(255);
		this->unschedule(schedule_selector(BattleScene::TypeWriter));
	}
}

void BattleScene::ResetAllState()
{
	this->m_stateBattleMessage = false;
	this->m_statePlayer = false;
	this->m_stateOpponent = false;
	this->m_player->SetState(false);
	this->m_opponent->SetState(false);
	this->SetButtonVisible(true);
	this->m_labelBattleLog->setString("What will you do?");
	this->BattleMessageSetOpacity(255);
}

Pokemon * BattleScene::GetTrainerPokemon()
{
	return this->m_player;
}

void BattleScene::PlayerAttackOpponent()
{
	Director::getInstance()->getEventDispatcher()->pauseEventListenersForTarget(this);
	this->m_stateBattleMessage = false;
	auto finished = CallFunc::create([this]() {
		this->m_player->Attack(this->m_opponent, this->m_playerSkill);
		this->schedule(schedule_selector(BattleScene::ReduceHpOpponent), 0.1);
	});
	auto jumpBy = JumpBy::create(0.5, Vec2::ZERO, 15, 2);
	this->m_player->GetSpriteBack()->runAction(Sequence::create(jumpBy, finished, nullptr));
}

void BattleScene::OpponentAttackPlayer()
{
	Director::getInstance()->getEventDispatcher()->pauseEventListenersForTarget(this);
	this->m_stateBattleMessage = false;
	auto finished = CallFunc::create([this]() {
		this->m_opponent->Attack(this->m_player, this->m_oppSkill);
		this->schedule(schedule_selector(BattleScene::ReduceHpPlayer), 0.1);
	});
	auto jumpBy = JumpBy::create(0.5, Vec2::ZERO, 15, 2);
	this->m_opponent->GetSpriteFront()->runAction(Sequence::create(jumpBy, finished, nullptr));
}

void BattleScene::DamageStepWithPlayerAttackFirst(float deltaTime)
{
	if (this->m_opponent->IsAlive() && this->m_player->IsAlive())
	{
		if (this->m_statePlayer == true && this->m_stateOpponent == true)
		{
			Director::getInstance()->getEventDispatcher()->pauseEventListenersForTarget(this);
			this->ResetAllState();
			this->unschedule(schedule_selector(BattleScene::DamageStepWithPlayerAttackFirst));
		}
		else
		{
			if (this->m_stateBattleMessage == true)
			{
				if (this->m_statePlayer == false)
				{
					this->PlayerAttackOpponent();
				}
				else
				{
					this->OpponentAttackPlayer();
				}
			}
			else
			{
				if (this->m_statePlayer == true && this->m_player->GetState() == true)
				{
					this->m_player->SetState(false);
					Director::getInstance()->getEventDispatcher()->resumeEventListenersForTarget(this);
					this->BattleMessage(this->m_opponent->GetName() + " used " + this->m_oppSkill->GetName() + ".");
				}
			}
		}
	}
	else
	{
		this->HasNextBattle();
		this->unschedule(schedule_selector(BattleScene::DamageStepWithPlayerAttackFirst));
	}
}

void BattleScene::DamageStepWithOpponentAttackFirst(float deltaTime)
{
	if (this->m_player->IsAlive() && this->m_opponent->IsAlive())
	{
		if (this->m_statePlayer == true && this->m_stateOpponent == true)
		{
			Director::getInstance()->getEventDispatcher()->pauseEventListenersForTarget(this);
			this->ResetAllState();
			this->unschedule(schedule_selector(BattleScene::DamageStepWithOpponentAttackFirst));
		}
		else
		{
			if (this->m_stateBattleMessage == true)
			{
				if (this->m_stateOpponent == false)
				{
					this->OpponentAttackPlayer();
				}
				else
				{
					this->PlayerAttackOpponent();
				}
			}
			else
			{
				if (this->m_stateOpponent == true && this->m_opponent->GetState() == true)
				{
					this->m_opponent->SetState(false);
					Director::getInstance()->getEventDispatcher()->resumeEventListenersForTarget(this);
					this->BattleMessage(this->m_player->GetName() + " used " + this->m_playerSkill->GetName() + ".");
				}
			}
		}
	}
	else
	{
		this->HasNextBattle();
		this->unschedule(schedule_selector(BattleScene::DamageStepWithOpponentAttackFirst));
	}
}

void BattleScene::RestoreHealthStep(float deltaTime)
{
	if (this->m_opponent->IsAlive() && this->m_player->IsAlive())
	{
		if (this->m_statePlayer == true && this->m_stateOpponent == true)
		{
			Director::getInstance()->getEventDispatcher()->pauseEventListenersForTarget(this);
			this->ResetAllState();
			this->unschedule(schedule_selector(BattleScene::RestoreHealthStep));
		}
		else
		{
			if (this->m_stateBattleMessage == true)
			{
				if (this->m_statePlayer == false)
				{
					Director::getInstance()->getEventDispatcher()->pauseEventListenersForTarget(this);
					this->m_stateBattleMessage = false;
					auto finished = CallFunc::create([this]() {
						this->m_statePlayer = true;
						this->m_player->SetState(true);
					});
					auto index = ((float)this->m_player->GetMaxHP() / this->m_player->GetCurrentHP());
					auto sq = Sequence::create(ScaleTo::create(0.5, scale_hpBar / index, this->m_hpPlayer->getScaleY()), finished, nullptr);
					this->m_hpPlayer->runAction(sq);
					auto audio = SimpleAudioEngine::getInstance();
					audio->playEffect("res/Sound/UseItemRecoverHP.mp3", false);
				}
				else
				{
					this->OpponentAttackPlayer();
				}
			}
			else
			{
				if (this->m_statePlayer == true && this->m_player->GetState() == true)
				{
					this->m_player->SetState(false);
					Director::getInstance()->getEventDispatcher()->resumeEventListenersForTarget(this);
					this->BattleMessage(this->m_opponent->GetName() + " used " + this->m_oppSkill->GetName() + ".");
				}
			}
		}
	}
	else
	{
		this->HasNextBattle();
		this->unschedule(schedule_selector(BattleScene::RestoreHealthStep));
	}
}

void BattleScene::ChangePokemonStep(float deltaTime)
{
	if (this->m_opponent->IsAlive() && this->m_player->IsAlive())
	{
		if (this->m_statePlayer == true && this->m_stateOpponent == true)
		{
			Director::getInstance()->getEventDispatcher()->pauseEventListenersForTarget(this);
			this->ResetAllState();
			this->unschedule(schedule_selector(BattleScene::ChangePokemonStep));
		}
		else
		{
			if (this->m_stateBattleMessage == true)
			{
				if (this->m_statePlayer == false)
				{
					Director::getInstance()->getEventDispatcher()->pauseEventListenersForTarget(this);
					this->m_stateBattleMessage = false;
					this->LoadPlayerPosition();
					this->m_pokeball->setPosition(this->m_player->GetPosition());
					this->m_pokeball->setVisible(true);
					auto pokeball_finished = CallFunc::create([this]() {
						this->m_pokeball->setVisible(false);
						auto finished = CallFunc::create([this]() {
							this->m_statePlayer = true;
							this->m_player->SetState(true);
							this->LoadPlayerHpBar();
						});
						auto scaleTo = ScaleTo::create(1, 2.5);
						this->m_player->GetSpriteBack()->runAction(Sequence::create(scaleTo, finished, nullptr));
					});
					this->m_pokeball->runAction(Sequence::create(Animate::create(this->m_animationPokeball), pokeball_finished, nullptr));
				}
				else
				{
					this->OpponentAttackPlayer();
				}
			}
			else
			{
				if (this->m_statePlayer == true && this->m_player->GetState() == true)
				{
					this->m_player->SetState(false);
					Director::getInstance()->getEventDispatcher()->resumeEventListenersForTarget(this);
					this->BattleMessage(this->m_opponent->GetName() + " used " + this->m_oppSkill->GetName() + ".");
				}
			}
		}
	}
	else
	{
		this->HasNextBattle();
		this->unschedule(schedule_selector(BattleScene::ChangePokemonStep));
	}
}

void BattleScene::ReduceHpPlayer(float deltaTime)
{
	if (this->m_opponent->GetState() == true)
	{
		auto fadeOut = FadeOut::create(0.1);
		auto fadeIn = FadeIn::create(0.1);
		auto fadeOut_In = Sequence::create(fadeOut, fadeIn, fadeOut->clone(), fadeIn->clone(), nullptr);
		this->m_player->GetSpriteBack()->runAction(fadeOut_In);
		//
		//
		if (this->m_player->GetCurrentHP() == 0)
		{
			this->m_hpPlayer->runAction(ScaleTo::create(0.5, 0));
		}
		else
		{
			auto finished = CallFunc::create([this]() {
				this->m_stateOpponent = true;
			});
			auto index = ((float)this->m_player->GetMaxHP() / this->m_player->GetCurrentHP());
			auto sq = Sequence::create(ScaleTo::create(0.5, scale_hpBar / index, this->m_hpPlayer->getScaleY()), finished, nullptr);
			this->m_hpPlayer->runAction(sq);
		}
		if (m_player->GetCurrentHP()*100/m_player->GetMaxHP()<20)
		{
			auto audio = SimpleAudioEngine::getInstance();
			audio->playEffect("LowHealth.mp3", true);
		}
		this->unschedule(schedule_selector(BattleScene::ReduceHpPlayer));
	}
}

void BattleScene::ReduceHpOpponent(float deltaTime)
{
	if (this->m_player->GetState() == true)
	{
		auto fadeOut = FadeOut::create(0.1);
		auto fadeIn = FadeIn::create(0.1);
		auto fadeOut_In = Sequence::create(fadeOut, fadeIn, fadeOut->clone(), fadeIn->clone(), nullptr);
		this->m_opponent->GetSpriteFront()->runAction(fadeOut_In);
		//
		//
		if (this->m_opponent->GetCurrentHP() == 0)
		{
			this->m_hpOpponent->runAction(ScaleTo::create(0.5, 0));
		}
		else
		{
			auto finished = CallFunc::create([this]() {
				this->m_statePlayer = true;
			});
			auto index = ((float)this->m_opponent->GetMaxHP() / this->m_opponent->GetCurrentHP());
			auto sq = Sequence::create(ScaleTo::create(0.5, scale_hpBar / index, this->m_hpOpponent->getScaleY()), finished, nullptr);
			this->m_hpOpponent->runAction(sq);
		}
		this->unschedule(schedule_selector(BattleScene::ReduceHpOpponent));
	}
}

void BattleScene::BattleMessageSetOpacity(GLubyte opacity)
{
	for (int i = 0; i < this->m_labelBattleLog->getStringLength(); i++)
	{
		auto letter = this->m_labelBattleLog->getLetter(i);
		if (letter != nullptr)
		{
			letter->setOpacity(opacity);
		}
	}
}

void BattleScene::LoadPlayerPosition()
{
	auto obj = this->m_tiledmap->getObjectGroup("pokemon");
	auto x = obj->getObject("player").at("x").asFloat();
	auto y = obj->getObject("player").at("y").asFloat();
	this->m_player->SetPosition(x * this->m_tiledmap->getScaleX(), y * this->m_tiledmap->getScaleY());
	this->addChild(this->m_player->GetSpriteBack(), 10);
	for (int i = 0; i < this->m_player->GetCountSkills(); i++)
	{
		this->addChild(this->m_player->GetSkillById(i)->GetSpriteFront(), 100);
		this->m_player->GetSkillById(i)->SetPosition(this->m_player->GetPosition());
	}
	this->m_levelUp->setPosition(x, y);
}

void BattleScene::LoadPlayerHpBar()
{
	this->m_labelPlayerName->setString(this->m_player->GetName());
	this->m_labelPlayerLevel->setString(to_string(this->m_player->GetLevel()));
	if (this->m_player->GetCurrentHP() == this->m_player->GetMaxHP())
	{
		this->m_hpPlayer->setScaleX(scale_hpBar);
	}
	else
	{
		auto index = ((float)this->m_player->GetMaxHP() / this->m_player->GetCurrentHP());
		this->m_hpPlayer->setScaleX(scale_hpBar / index);
	}
	this->m_hpPlayer->setScaleY(0.12);
	this->m_hpPlayer->setVisible(true);
}

void BattleScene::LoadOpponentPosition()
{
	auto obj = this->m_tiledmap->getObjectGroup("pokemon");
	auto x = obj->getObject("opponent").at("x").asFloat();
	auto y = obj->getObject("opponent").at("y").asFloat();
	this->m_opponent->SetPosition(x * this->m_tiledmap->getScaleX(), y * this->m_tiledmap->getScaleY());
	this->addChild(this->m_opponent->GetSpriteFront(), 10);
	//
	//
	for (int i = 0; i < this->m_opponent->GetCountSkills(); i++)
	{
		this->addChild(this->m_opponent->GetSkillById(i)->GetSpriteFront(), 100);
		this->m_opponent->GetSkillById(i)->SetPosition(this->m_opponent->GetPosition());
	}
}

void BattleScene::LoadOpponentHpBar()
{
	this->m_labelOppName->setString(this->m_opponent->GetName());
	this->m_labelOppLevel->setString(to_string(this->m_opponent->GetLevel()));
	this->m_hpOpponent->setScaleX(scale_hpBar);
	this->m_hpOpponent->setScaleY(0.12);
	this->m_hpOpponent->setVisible(true);
}

void BattleScene::InitTiledMap()
{
	auto visibleSize = Director::getInstance()->getVisibleSize();

	this->m_tiledmap = ResourceManager::GetInstance()->GetTiledMapById(0);
	auto scale_x = visibleSize.width / this->m_tiledmap->getContentSize().width;
	auto scale_y = visibleSize.height / this->m_tiledmap->getContentSize().height;
	this->m_tiledmap->setScaleX(scale_x);
	this->m_tiledmap->setScaleY(scale_y);
	this->addChild(this->m_tiledmap, -10);

	this->m_background = ResourceManager::GetInstance()->GetSpriteById(132);
	scale_x = visibleSize.width / this->m_background->getContentSize().width;
	scale_y = visibleSize.height / this->m_background->getContentSize().height;
	this->m_background->setScaleX(scale_x);
	this->m_background->setScaleY(scale_y);
	this->m_background->setPosition(visibleSize.width / 2, visibleSize.height / 2);
	this->addChild(this->m_background, -100);
}

void BattleScene::InitUI()
{
	auto visibleSize = Director::getInstance()->getVisibleSize();
	this->m_buttonFight = ResourceManager::GetInstance()->GetButtonById(5);
	this->m_buttonFight->setAnchorPoint(Vec2(1, 0.5));
	this->m_buttonFight->setPosition(Vec2(visibleSize.width, this->m_buttonFight->getContentSize().height * 4));
	this->m_buttonFight->setTag(0);

	this->m_buttonBag = ResourceManager::GetInstance()->GetButtonById(5);
	this->m_buttonBag->setAnchorPoint(Vec2(1, 0.5));
	this->m_buttonBag->setPosition(Vec2(visibleSize.width, this->m_buttonBag->getContentSize().height * 2.9));
	this->m_buttonBag->setTag(1);

	this->m_buttonPokemon = ResourceManager::GetInstance()->GetButtonById(5);
	this->m_buttonPokemon->setAnchorPoint(Vec2(1, 0.5));
	this->m_buttonPokemon->setPosition(Vec2(visibleSize.width, this->m_buttonPokemon->getContentSize().height * 1.8));
	this->m_buttonPokemon->setTag(2);

	this->m_buttonRun = ResourceManager::GetInstance()->GetButtonById(5);
	this->m_buttonRun->setAnchorPoint(Vec2(1, 0.5));
	this->m_buttonRun->setPosition(Vec2(visibleSize.width, this->m_buttonRun->getContentSize().height * 0.7));
	this->m_buttonRun->setTag(3);

	this->addChild(this->m_buttonFight, 0);
	this->addChild(this->m_buttonBag, 0);
	this->addChild(this->m_buttonPokemon, 0);
	this->addChild(this->m_buttonRun, 0);

	auto x = this->m_buttonFight->getContentSize().width / 2;
	auto y = this->m_buttonFight->getContentSize().height / 2;

	this->m_labelSkill1 = ResourceManager::GetInstance()->GetLabelById(0);
	this->m_labelSkill1->setTag(0);
	this->m_labelSkill1->setPosition(x, y);

	this->m_labelSkill2 = ResourceManager::GetInstance()->GetLabelById(0);
	this->m_labelSkill2->setTag(1);
	this->m_labelSkill2->setPosition(x, y);

	this->m_labelSkill3 = ResourceManager::GetInstance()->GetLabelById(0);
	this->m_labelSkill3->setTag(2);
	this->m_labelSkill3->setPosition(x, y);

	this->m_labelSkill4 = ResourceManager::GetInstance()->GetLabelById(0);
	this->m_labelSkill4->setTag(3);
	this->m_labelSkill4->setPosition(x, y);
	//
	//
	this->m_messageBox = ResourceManager::GetInstance()->GetSpriteById(130);
	auto scale_x = 0.7;
	auto scale_y = 0.7;
	this->m_messageBox->setScaleX(scale_x);
	this->m_messageBox->setScaleY(scale_y);
	this->m_messageBox->setPosition(visibleSize.width / 1.88, this->m_buttonFight->getContentSize().height * 2);
	this->addChild(this->m_messageBox, 0);
	this->m_labelBattleLog = ResourceManager::GetInstance()->GetLabelById(0);
	this->m_labelBattleLog->setAnchorPoint(Vec2::ZERO);
	this->m_labelBattleLog->setScale(1.5);
	this->m_labelBattleLog->setTextColor(Color4B::BLACK);
	this->m_labelBattleLog->setPosition(this->m_messageBox->getContentSize().width * scale_x / 10, this->m_messageBox->getContentSize().height * scale_y / 1.2);
	this->m_messageBox->addChild(this->m_labelBattleLog);
	//
	//
	this->m_buttonFight->addChild(this->m_labelSkill1);
	this->m_buttonBag->addChild(this->m_labelSkill2);
	this->m_buttonPokemon->addChild(this->m_labelSkill3);
	this->m_buttonRun->addChild(this->m_labelSkill4);
	this->SetButtonVisible(false);
	//
	//
	this->m_labelOppName = ResourceManager::GetInstance()->GetLabelById(0);
	this->m_labelOppName->setTextColor(Color4B::BLACK);
	this->m_labelOppName->setAnchorPoint(Vec2::ZERO);
	this->m_labelOppName->setScale(0.8);
	this->m_labelOppLevel = ResourceManager::GetInstance()->GetLabelById(0);
	this->m_labelOppLevel->setTextColor(Color4B::BLACK);
	this->m_hpOpponent = ResourceManager::GetInstance()->GetSpriteById(131);
	this->m_hpOpponent->setAnchorPoint(Vec2::ZERO);
	this->m_hpOpponent->setVisible(false);
	auto obj = this->m_tiledmap->getObjectGroup("opponent");
	x = obj->getObject("name").at("x").asFloat();
	y = obj->getObject("name").at("y").asFloat();
	this->m_labelOppName->setPosition(x * this->m_tiledmap->getScaleX(), y * this->m_tiledmap->getScaleY());
	this->addChild(this->m_labelOppName, 0);
	x = obj->getObject("level").at("x").asFloat();
	y = obj->getObject("level").at("y").asFloat();
	this->m_labelOppLevel->setPosition(x * this->m_tiledmap->getScaleX(), y * this->m_tiledmap->getScaleY());
	this->addChild(this->m_labelOppLevel, 0);
	x = obj->getObject("hp").at("x").asFloat();
	y = obj->getObject("hp").at("y").asFloat();
	this->m_hpOpponent->setPosition(x * this->m_tiledmap->getScaleX(), y * this->m_tiledmap->getScaleY());
	this->addChild(this->m_hpOpponent, 0);
	//
	//
	this->m_labelPlayerName = ResourceManager::GetInstance()->GetLabelById(0);
	this->m_labelPlayerName->setTextColor(Color4B::BLACK);
	this->m_labelPlayerName->setAnchorPoint(Vec2::ZERO);
	this->m_labelPlayerName->setScale(0.8);
	this->m_labelPlayerLevel = ResourceManager::GetInstance()->GetLabelById(0);
	this->m_labelPlayerLevel->setTextColor(Color4B::BLACK);
	this->m_hpPlayer = ResourceManager::GetInstance()->GetSpriteById(131);
	this->m_hpPlayer->setAnchorPoint(Vec2::ZERO);
	this->m_hpPlayer->setVisible(false);
	obj = this->m_tiledmap->getObjectGroup("player");
	x = obj->getObject("name").at("x").asFloat();
	y = obj->getObject("name").at("y").asFloat();
	this->m_labelPlayerName->setPosition(x * this->m_tiledmap->getScaleX(), y * this->m_tiledmap->getScaleY());
	this->addChild(this->m_labelPlayerName, 0);
	x = obj->getObject("level").at("x").asFloat();
	y = obj->getObject("level").at("y").asFloat();
	this->m_labelPlayerLevel->setPosition(x * this->m_tiledmap->getScaleX(), y * this->m_tiledmap->getScaleY());
	this->addChild(this->m_labelPlayerLevel, 0);
	x = obj->getObject("hp").at("x").asFloat();
	y = obj->getObject("hp").at("y").asFloat();
	this->m_hpPlayer->setPosition(x * this->m_tiledmap->getScaleX(), y * this->m_tiledmap->getScaleY());
	this->addChild(this->m_hpPlayer, 0);
}

void BattleScene::InitObject()
{
	if (!Bag::GetInstance()->GetListPokemon().at(0)->IsAlive())
	{
		for (int i = 1; i < 6; i++)
		{
			if (Bag::GetInstance()->GetListPokemon().at(i) != nullptr)
			{
				if (Bag::GetInstance()->GetListPokemon().at(i)->IsAlive())
				{
					Bag::GetInstance()->ChangePokemon(i);
					break;
				}
			}
		}
	}
	this->m_player = Bag::GetInstance()->GetListPokemon().at(0);
	this->m_player->RemoveFromParent();
	this->m_player->SetScale(0);
	this->m_opponent = listOpponentPokemon.at(0);
	this->m_opponent->SetScale(2.5);
	this->LoadOpponentPosition();
	this->LoadOpponentHpBar();

	this->m_pokeball = ResourceManager::GetInstance()->GetSpriteById(147);
	this->m_pokeball->setAnchorPoint(Vec2(0.5, 0));
	this->m_pokeball->setScale(1.5);
	this->m_pokeball->setVisible(false);
	this->m_animationPokeball = ResourceManager::GetInstance()->GetAnimationById(122);
	this->m_animationPokeball->setDelayPerUnit(0.1);
	this->m_animationPokeball->setRestoreOriginalFrame(true);
	this->addChild(this->m_pokeball, 10);

	this->m_levelUp = ResourceManager::GetInstance()->GetSpriteById(148);
	this->m_levelUp->setScale(0.4);
	this->m_levelUp->setAnchorPoint(Vec2(0.25, 0));
	this->m_levelUp->setVisible(false);
	this->addChild(this->m_levelUp, 100);
}

void BattleScene::AddEventListener()
{
	this->m_buttonFight->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
		switch (type)
		{
		case cocos2d::ui::Widget::TouchEventType::BEGAN:
			break;
		case cocos2d::ui::Widget::TouchEventType::ENDED:
			if (this->m_labelSkill1->getString() == "Fight")
			{
				this->m_labelSkill1->setString(this->m_player->GetSkillById(0) != nullptr ? this->m_player->GetSkillById(0)->GetName() : "-");
				this->m_labelSkill2->setString(this->m_player->GetSkillById(1) != nullptr ? this->m_player->GetSkillById(1)->GetName() : "-");
				this->m_labelSkill3->setString(this->m_player->GetSkillById(2) != nullptr ? this->m_player->GetSkillById(2)->GetName() : "-");
				this->m_labelSkill4->setString("Back");
			}
			else if (this->m_labelSkill1->getString() != "-")
			{
				int choice = ((Button*)sender)->getTag();
				if (this->m_player->GetSkillById(choice)->GetCurrentPP() > 0)
				{
					this->SetButtonVisible(false);
					this->BattlePhase(choice);
				}
				else
				{
					this->m_labelBattleLog->setString("No PP!");
				}
			}
			break;
		default:
			break;
		}
	});
	this->m_buttonBag->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
		switch (type)
		{
		case cocos2d::ui::Widget::TouchEventType::BEGAN:
			break;
		case cocos2d::ui::Widget::TouchEventType::ENDED:
			if (this->m_labelSkill2->getString() == "Bag")
			{
				UICustom::Popup* popupPokemon = UICustom::Popup::CreateBagItemInBattle();
				this->addChild(popupPokemon, 1000);
				this->SetButtonVisible(false);
			}
			else if (this->m_labelSkill2->getString() != "-")
			{
				int choice = ((Button*)sender)->getTag();
				if (this->m_player->GetSkillById(choice)->GetCurrentPP() > 0)
				{
					this->SetButtonVisible(false);
					this->BattlePhase(choice);
				}
				else
				{
					this->m_labelBattleLog->setString("No PP!");
				}
			}
			break;
		default:
			break;
		}
	});
	this->m_buttonPokemon->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
		switch (type)
		{
		case cocos2d::ui::Widget::TouchEventType::BEGAN:
			break;
		case cocos2d::ui::Widget::TouchEventType::ENDED:
			if (this->m_labelSkill3->getString() == "Pokemon")
			{
				UICustom::Popup* popupPokemon = UICustom::Popup::CreateBagInBattle();
				this->addChild(popupPokemon, 1000);
			}
			else if (this->m_labelSkill3->getString() != "-")
			{
				int choice = ((Button*)sender)->getTag();
				if (this->m_player->GetSkillById(choice)->GetCurrentPP() > 0)
				{
					this->SetButtonVisible(false);
					this->BattlePhase(choice);
				}
				else
				{
					this->m_labelBattleLog->setString("No PP!");
				}
			}
			break;
		default:
			break;
		}
	});
	this->m_buttonRun->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
		switch (type)
		{
		case cocos2d::ui::Widget::TouchEventType::BEGAN:
			break;
		case cocos2d::ui::Widget::TouchEventType::ENDED:
			if (this->m_labelSkill4->getString() == "Run")
			{
				if (countOppPokemons < 2)
				{
					this->ReleaseChildren();
					this->getParent()->scheduleUpdate();
					Buttons::GetIntance()->SetVisible(true);
					this->removeFromParent();
					auto audio = SimpleAudioEngine::getInstance();
					audio->playBackgroundMusic("res/Sound/Town.mp3", true);
				}
			}
			else
			{
				this->m_labelBattleLog->setString("What will you do?");
				this->SetButtonVisible(true);
			}
			break;
		default:
			break;
		}
	});

	auto keyboardListener = EventListenerKeyboard::create();
	keyboardListener->onKeyPressed = CC_CALLBACK_2(BattleScene::onKeyPressed, this);
	keyboardListener->onKeyReleased = CC_CALLBACK_2(BattleScene::onKeyReleased, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, this);

	auto touchListener = EventListenerTouchOneByOne::create();
	touchListener->onTouchBegan = CC_CALLBACK_2(BattleScene::onTouchBegan, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);

	Director::getInstance()->getEventDispatcher()->pauseEventListenersForTarget(this);
}

void BattleScene::onKeyPressed(EventKeyboard::KeyCode keyCode, Event * e)
{
	switch (keyCode)
	{
	case EventKeyboard::KeyCode::KEY_Z:
		if (this->m_labelBattleLog->getOpacity() == 0)
		{
			this->unschedule(schedule_selector(BattleScene::TypeWriter));
			this->BattleMessageSetOpacity(255);
			this->m_labelBattleLog->setOpacity(255);
		}
		else
		{
			this->m_stateBattleMessage = true;
		}
	default:
		break;
	}
}

void BattleScene::onKeyReleased(EventKeyboard::KeyCode keyCode, Event * e)
{
	switch (keyCode)
	{
	default:
		break;
	}
}

bool BattleScene::onTouchBegan(Touch * touch, Event * e)
{
	if (this->m_labelBattleLog->getOpacity() == 0)
	{
		this->unschedule(schedule_selector(BattleScene::TypeWriter));
		this->BattleMessageSetOpacity(255);
		this->m_labelBattleLog->setOpacity(255);
	}
	else
	{
		this->m_stateBattleMessage = true;
	}
	return true;
}

void BattleScene::BattleMessage(string logg)
{
	this->m_labelBattleLog->setString(logg);
	this->BattleMessageSetOpacity(0);
	this->m_labelBattleLog->setOpacity(0);
	writing = 0;
	this->schedule(schedule_selector(BattleScene::TypeWriter), 0.1);
}

void BattleScene::SetButtonVisible(bool visible)
{
	this->m_labelSkill1->setString("Fight");
	this->m_labelSkill2->setString("Bag");
	this->m_labelSkill3->setString("Pokemon");
	this->m_labelSkill4->setString("Run");
	this->m_buttonFight->setVisible(visible);
	this->m_buttonBag->setVisible(visible);
	this->m_buttonPokemon->setVisible(visible);
	this->m_buttonRun->setVisible(visible);
}

void BattleScene::StartBattle()
{
	auto listener = CallFunc::create([this]() {
		if (this->m_stateBattleMessage == true)
		{
			Director::getInstance()->getEventDispatcher()->pauseEventListenersForTarget(this);
			this->m_stateBattleMessage = false;
			this->LoadPlayerPosition();
			this->m_pokeball->setPosition(this->m_player->GetPosition());
			this->m_pokeball->setVisible(true);
			auto pokeball_finished = CallFunc::create([this]() {
				this->m_pokeball->setVisible(false);
				auto finished = CallFunc::create([this]() {
					this->LoadPlayerHpBar();
					this->ResetAllState();
				});
				auto scaleTo = ScaleTo::create(1, 2.5);
				this->m_player->GetSpriteBack()->runAction(Sequence::create(scaleTo, finished, nullptr));
			});
			this->m_pokeball->runAction(Sequence::create(Animate::create(this->m_animationPokeball), pokeball_finished, nullptr));
			this->stopActionByTag(100);
		}
	});
	auto rp = RepeatForever::create(Spawn::create(listener, nullptr));
	rp->setTag(100);
	this->runAction(rp);
	Director::getInstance()->getEventDispatcher()->resumeEventListenersForTarget(this);
	this->BattleMessage("Go, " + this->m_player->GetName() + "!");
}

void BattleScene::HasNextBattle()
{
	if (this->m_player->IsAlive() == false)
	{
		auto listener = CallFunc::create([this]() {
			if (this->m_stateBattleMessage == true)
			{
				Director::getInstance()->getEventDispatcher()->pauseEventListenersForTarget(this);
				this->m_stateBattleMessage = false;
				if (Bag::GetInstance()->GetCountPokemon() > 0)
				{
					this->StandByPhase();
				}
				else
				{
					this->EndBattle();
				}
				this->stopActionByTag(200);
			}
		});
		auto rp = RepeatForever::create(Spawn::create(listener, nullptr));
		rp->setTag(200);
		this->runAction(rp);
		auto finished = CallFunc::create([this]() {
			Director::getInstance()->getEventDispatcher()->resumeEventListenersForTarget(this);
			this->BattleMessage(this->m_player->GetName() + " fainted.");
		});
		auto scaleTo = ScaleTo::create(1, 0);
		this->m_player->GetSpriteBack()->runAction(Sequence::create(scaleTo, finished, nullptr));
	}
	else
	{
		auto listener = CallFunc::create([this]() {
			if (this->m_stateBattleMessage == true)
			{
				Director::getInstance()->getEventDispatcher()->pauseEventListenersForTarget(this);
				this->m_stateBattleMessage = false;
				if (this->m_player->GetLevel() - stoi(this->m_labelPlayerLevel->getString()) > 0)
				{
					auto audio = SimpleAudioEngine::getInstance();
					audio->playEffect("res/Sound/LevelUp.mp3", false);
					this->m_levelUp->setVisible(true);
					this->m_levelUp->runAction(Sequence::create(DelayTime::create(1.5), CallFunc::create([this]() {
						this->m_player->RemoveFromParent();
						this->LoadPlayerPosition();
						this->LoadPlayerHpBar();
						this->m_levelUp->setVisible(false);
					}), nullptr));
				}
				if (countOppPokemons < 2)
				{
					listOpponentPokemon.erase(listOpponentPokemon.begin());
					Bag::GetInstance()->AddPokemon(this->m_opponent);
					this->m_opponent->RemoveFromParent();
				}
				else
				{
					this->m_opponent->RemoveFromParent();
					this->m_opponent->Restore();
					listOpponentPokemon.erase(listOpponentPokemon.begin());
				}
				if (listOpponentPokemon.size() > 0)
				{
					this->OpponentChangePokemon();
				}
				else
				{
					this->EndBattle();
				}
				this->stopActionByTag(200);
			}
		});
		auto rp = RepeatForever::create(Spawn::create(listener, nullptr));
		rp->setTag(200);
		this->runAction(rp);
		auto finished = CallFunc::create([this]() {
			Director::getInstance()->getEventDispatcher()->resumeEventListenersForTarget(this);
			int xp = this->m_player->GetCurrentExp() + (this->m_opponent->GetLevel() * 2);
			this->m_player->SetCurrentExp(xp);
			this->BattleMessage(this->m_opponent->GetName() + " fainted. " + this->m_player->GetName() + " gain "
				+ to_string((this->m_opponent->GetLevel() * 2)) + " exp.");
		});
		auto scaleTo = ScaleTo::create(1, 0);
		this->m_opponent->GetSpriteFront()->runAction(Sequence::create(scaleTo, finished, nullptr));
	}
}

void BattleScene::BattlePhase(int idSkill)
{
	if (this->m_player->GetAtkSpeed() >= this->m_opponent->GetAtkSpeed())
	{
		this->m_playerSkill = this->m_player->GetSkillById(idSkill);
		int id = rand() % this->m_opponent->GetCountSkills();
		this->m_oppSkill = this->m_opponent->GetSkillById(id);
		Director::getInstance()->getEventDispatcher()->resumeEventListenersForTarget(this);
		this->BattleMessage(this->m_player->GetName() + " used " + this->m_playerSkill->GetName() + ".");
		this->schedule(schedule_selector(BattleScene::DamageStepWithPlayerAttackFirst), 0.1);
	}
	else
	{
		int id = rand() % this->m_opponent->GetCountSkills();
		this->m_oppSkill = this->m_opponent->GetSkillById(id);
		this->m_playerSkill = this->m_player->GetSkillById(idSkill);
		Director::getInstance()->getEventDispatcher()->resumeEventListenersForTarget(this);
		this->BattleMessage(this->m_opponent->GetName() + " used " + this->m_oppSkill->GetName() + ".");
		this->schedule(schedule_selector(BattleScene::DamageStepWithOpponentAttackFirst), 0.1);
	}
}

void BattleScene::UseItem()
{
	int id = rand() % this->m_opponent->GetCountSkills();
	this->m_oppSkill = this->m_opponent->GetSkillById(id);
	Director::getInstance()->getEventDispatcher()->resumeEventListenersForTarget(this);
	this->BattleMessage(this->m_player->GetName() + " restore health.");
	this->schedule(schedule_selector(BattleScene::RestoreHealthStep), 0.1);
}

void BattleScene::TrainerChangePokemon()
{
	if (this->m_player->IsAlive() && this->m_opponent->IsAlive())
	{
		int id = rand() % this->m_opponent->GetCountSkills();
		this->m_oppSkill = this->m_opponent->GetSkillById(id);
		auto listener = CallFunc::create([this]() {
			if (this->m_stateBattleMessage == true)
			{
				Director::getInstance()->getEventDispatcher()->pauseEventListenersForTarget(this);
				this->m_stateBattleMessage = false;
				auto finished = CallFunc::create([this]() {
					this->m_player->RemoveFromParent();
					this->m_player = Bag::GetInstance()->GetListPokemon().at(0);
					this->m_player->RemoveFromParent();
					this->m_player->SetScale(0);
					Director::getInstance()->getEventDispatcher()->resumeEventListenersForTarget(this);
					this->BattleMessage("Go, " + this->m_player->GetName() + "!");
					this->schedule(schedule_selector(BattleScene::ChangePokemonStep), 0.1);
				});
				auto scaleTo = ScaleTo::create(1, 0);
				this->m_player->GetSpriteBack()->runAction(Sequence::create(scaleTo, finished, nullptr));
				this->stopActionByTag(1000);
			}
		});
		auto rp = RepeatForever::create(Spawn::create(listener, nullptr));
		rp->setTag(1000);
		this->runAction(rp);
		Director::getInstance()->getEventDispatcher()->resumeEventListenersForTarget(this);
		this->BattleMessage("Come back, " + this->m_player->GetName() + "!");
	}
	else
	{
		this->m_player->RemoveFromParent();
		this->m_player = Bag::GetInstance()->GetListPokemon().at(0);
		this->m_player->RemoveFromParent();
		this->m_player->SetScale(0);
	}
}

void BattleScene::OpponentChangePokemon()
{
	this->m_opponent = listOpponentPokemon.at(0);
	this->m_opponent->SetScale(0);
	auto listener = CallFunc::create([this]() {
		if (this->m_stateBattleMessage == true)
		{
			Director::getInstance()->getEventDispatcher()->pauseEventListenersForTarget(this);
			this->m_stateBattleMessage = false;
			this->LoadOpponentPosition();
			this->m_pokeball->setPosition(this->m_opponent->GetPosition());
			this->m_pokeball->setVisible(true);
			auto pokeball_finished = CallFunc::create([this]() {
				this->m_pokeball->setVisible(false);
				auto finished = CallFunc::create([this]() {
					this->LoadOpponentHpBar();
					this->ResetAllState();
				});
				auto scaleTo = ScaleTo::create(1, 2.5);
				this->m_opponent->GetSpriteFront()->runAction(Sequence::create(scaleTo, finished, nullptr));
			});
			this->m_pokeball->runAction(Sequence::create(Animate::create(this->m_animationPokeball), pokeball_finished, nullptr));
			this->stopActionByTag(2000);
		}
	});
	auto rp = RepeatForever::create(Spawn::create(listener, nullptr));
	rp->setTag(2000);
	this->runAction(rp);
	Director::getInstance()->getEventDispatcher()->resumeEventListenersForTarget(this);
	this->BattleMessage("Opponent used " + this->m_opponent->GetName() + ".");
}

void BattleScene::StandByPhase()
{
	UICustom::Popup* popupPokemon = UICustom::Popup::CreateBagInBattle();
	this->addChild(popupPokemon, 1000);
}

void BattleScene::EndBattle()
{
	auto listener = CallFunc::create([this]() {
		if (this->m_stateBattleMessage == true)
		{
			Director::getInstance()->getEventDispatcher()->pauseEventListenersForTarget(this);
			this->m_stateBattleMessage = false;
			this->ReleaseChildren();
			Bag::GetInstance()->CheckPokemonEvolve();
			this->getParent()->scheduleUpdate();
			Buttons::GetIntance()->SetVisible(true);
			this->removeFromParent();
			auto audio = SimpleAudioEngine::getInstance();
			audio->playBackgroundMusic("res/Sound/Town.mp3", true);
		}
	});
	auto rp = RepeatForever::create(Spawn::create(listener, nullptr));
	this->runAction(rp);
	if (this->m_player->IsAlive() == false)
	{
		Director::getInstance()->getEventDispatcher()->resumeEventListenersForTarget(this);
		this->BattleMessage("You lose!");
		this->m_opponent->Restore();
	}
	else
	{
		Director::getInstance()->getEventDispatcher()->resumeEventListenersForTarget(this);
		this->BattleMessage("You win!");
		this->getParent()->setTag(10);
	}
}