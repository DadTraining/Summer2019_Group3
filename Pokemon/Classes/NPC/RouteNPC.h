#pragma once

#include "MyObject.h"
#include "Pokemon.h"
#include <vector>
#include "NPC.h"

using namespace std;

class RouteNPC : public NPC
{
private:
	string name;
	vector<Pokemon*> m_pokemons;
public:
	RouteNPC();
	~RouteNPC();
	void Init() override;
	void Update(float deltaTime) override;
	vector<Pokemon*> GetListPokemon();
};