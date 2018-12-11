#pragma once
#include "../CSC8503Common/GameObject.h"
#include "../CSC8503Common/StateMachine.h"

using namespace NCL::CSC8503;

class RobotObject :
	public GameObject
{
public:
	RobotObject(int objID, std::string name);
	~RobotObject();

	void SetupStateMachine();
	StateMachine* GetStateMachine() { return robotMachine; }

	virtual void OnCollisionBegin(GameObject* otherObject) override;


protected:
	StateMachine* robotMachine;
};

