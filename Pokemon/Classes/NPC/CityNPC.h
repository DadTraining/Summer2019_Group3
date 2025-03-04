#pragma once

#include "MyObject.h"
#include "Pokemon.h"
#include <vector>
#include "NPC.h"

using namespace std;

class CityNPC : public NPC
{
private:
	string name;
	vector<Pokemon*> m_pokemons;
public:
	CityNPC();
	~CityNPC();
	void Update(float deltaTime) override;
};

