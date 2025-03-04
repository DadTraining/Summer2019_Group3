#include "Swellow.h"
#define hp 60
#define atk 85
#define def 60
#define speed 125

Swellow::Swellow()
{
}

Swellow::Swellow(int level) : Pokemon(level)
{
	this->Init(86, 87);
	//
	this->m_name = "Swellow";
	this->m_type = MyObject::TYPE_FLYING;
	this->LearnSkill(new Peck());
	this->LearnSkill(new Gust());
	this->LearnSkill(new Fly());
	this->m_maxHealth += hp;
	this->m_currentHealth = this->m_maxHealth;
	this->m_attack += atk;
	this->m_defense += def;
	this->m_attackSpeed += speed;
	this->m_maxExp += 2;
}

Swellow::Swellow(Taillow * it)
{
	this->Init(86, 87);
	//
	this->m_name = "Swellow";
	this->m_type = it->GetType();
	this->m_listSkill = { it->GetSkillById(0), it->GetSkillById(1), it->GetSkillById(2) };
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

Swellow::~Swellow()
{
}