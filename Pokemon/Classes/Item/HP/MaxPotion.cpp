#include "MaxPotion.h"

MaxPotion::MaxPotion()
{
	this->Init(107);
	this->name = "Posion";
	this->gold = 25;
	this->Number = 0;
}

MaxPotion::~MaxPotion()
{
}

void MaxPotion::ReviceHealth(Pokemon * pokemon)
{
	pokemon->SetCurrentHP(pokemon->GetMaxHP());
}