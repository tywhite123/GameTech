#pragma once
#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/GameObject.h"

using namespace NCL::CSC8503;

class SpinnerObject : public GameObject
{
public:
	SpinnerObject(std::string name);
	~SpinnerObject();

	StateMachine* GetStateMachine() { return spinnerMachine; }
	void SetupStateMachine();


protected:
	StateMachine* spinnerMachine;
};

