#pragma once
#include"cocos2d.h"
#include "Skill.h"

class FireBlast : public Skill
{
public:
	FireBlast();
	~FireBlast();
	void Init();
	void Run(Vec2 position) override;
};