#include "Ralts.h"
#include "Kirlia.h"
#define hp 35
#define atk 30
#define def 30
#define speed 40

Ralts::Ralts()
{
}

Ralts::Ralts(int level) : Pokemon(level)
{
	this->Init(80, 81);
	//
	this->m_name = "Ralts";
	this->m_type = MyObject::TYPE_GRASS;
	this->LearnSkill(new LeechSeed());
	this->m_maxHealth += hp;
	this->m_currentHealth = this->m_maxHealth;
	this->m_attack += atk;
	this->m_defense += def;
	this->m_attackSpeed += speed;
	this->m_maxExp += 2;
}

Ralts::~Ralts()
{
}

Pokemon * Ralts::Evolve()
{
	if (this->m_level >= 7)
	{
		this->LearnSkill(new EnergyBall());
		this->m_evolved = true;
		return new Kirlia(this);
	}
	else
	{
		return nullptr;
	}
}
