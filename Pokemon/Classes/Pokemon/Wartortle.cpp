#include "Wartortle.h"
#include "Blastoise.h"
#define hp 59
#define atk 64
#define def 80
#define speed 58

Wartortle::Wartortle()
{
	this->Init(56, 57);
}

Wartortle::Wartortle(Squirtle * it)
{
	this->Init(56, 57);
	//
	this->m_name = "Wartortle";
	this->m_level = it->GetLevel();
	this->m_maxHealth = it->GetMaxHP() + 15;
	this->m_currentHealth = this->m_maxHealth;
	this->m_attack = it->GetAtk() + 5;
	this->m_defense = it->GetDef() + 5;
	this->m_attackSpeed = it->GetAtkSpeed() + 2;
	this->m_currentExp = 0;
	this->m_maxExp = it->GetMaxExp();
	delete it;
}

Wartortle::~Wartortle()
{
}

Pokemon * Wartortle::Evolve()
{
	if (this->m_level >= 15)
	{
		return new Blastoise(this);
	}
	else
	{
		return nullptr;
	}
}