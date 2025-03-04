#include "Charmander.h"
#include "Charmeleon.h"
#define hp 40
#define atk 52
#define def 43
#define speed 65


Charmander::Charmander()
{
}

Charmander::Charmander(int level) : Pokemon(level)
{
	this->Init(4, 5);
	//
	this->m_name = "Charmander";
	this->m_type = MyObject::TYPE_FIRE;
	this->LearnSkill(new Ember());
	this->m_maxHealth += hp;
	this->m_currentHealth = this->m_maxHealth;
	this->m_attack += atk;
	this->m_defense += def;
	this->m_attackSpeed += speed;
	this->m_maxExp += 2;
}


Charmander::~Charmander()
{
}

Pokemon * Charmander::Evolve()
{
	if (this->m_level >= 8)
	{
		this->LearnSkill(new FireBlast());
		this->m_evolved = true;
		return new Charmeleon(this);
	}
	else
	{
		return nullptr;
	}
}