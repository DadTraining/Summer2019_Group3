#include "Model.h"

bool Model::ENTEI = true;
bool Model::SUICUNE = true;
bool Model::RAIKOU = true;
bool Model::ROUTE1NPC = true;
bool Model::CAVENPC = true;
bool Model::LAKENPC = true;
bool Model::ROUTE2NPC = true;
bool Model::ROADNPC = true;
bool Model::CHAMPIONNPC = true;
bool Model::DOCTOR = true;
Model::Model() {

}

Model::~Model() {

}

int Model::stateGame = 0;

string Model::GetTipsGame()
{
	if (stateGame == 0) {
		return "You must go to the Doctor's House to get pokemon! If you have not Pokemon, you couldn't battle with NPC";
	}
	else if (stateGame == 1) {
		return "You can go to the grass to get pokemon or battle with npc!";
	}
	else
		return "tai oc cho qua di";
}
void Model::SetStateGame(int state) {
	if (state > stateGame) {
		stateGame = state;
	}
	else return;
}