#pragma once
#include "GameObject.h"

using namespace NCL::CSC8503;

class PlayerObject :
	public GameObject
{
public:
	PlayerObject(string name);
	virtual ~PlayerObject();

	virtual void OnCollisionBegin(GameObject* otherObject) override;
	//virtual void OnCollisionEnd(GameObject* otherObject) override;

};

