#pragma once
#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/GameObject.h"

using namespace NCL::CSC8503;

class SpinnerObject : public GameObject
{
public:
	SpinnerObject(int objID, std::string name, float spin);
	~SpinnerObject();

	StateMachine* GetStateMachine() { return spinnerMachine; }
	void SetupStateMachine();


protected:
	StateMachine* spinnerMachine;
	float spinVal;

};

