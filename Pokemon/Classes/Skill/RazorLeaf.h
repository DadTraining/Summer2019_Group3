#pragma once
#include"cocos2d.h"
#include "Skill.h"

class RazorLeaf : public Skill
{
public:
	RazorLeaf();
	~RazorLeaf();
	void Init();
	void Run(Vec2 position) override;
};
