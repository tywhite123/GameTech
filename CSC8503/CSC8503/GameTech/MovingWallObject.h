#pragma once

#include "../CSC8503Common/GameObject.h"
#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/State.h"

using namespace NCL::CSC8503;

class MovingWallObject : public GameObject
{
public:
	MovingWallObject(string name);
	~MovingWallObject();

	StateMachine* GetStateMachine() { return moveMachine; }
	void SetupStateMachine();


private:
	StateMachine* moveMachine;

	float z;
	float y;
	float x;

};

