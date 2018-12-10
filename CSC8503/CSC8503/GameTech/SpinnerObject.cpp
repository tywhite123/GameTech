#include "SpinnerObject.h"
#include "../CSC8503Common/State.h"


SpinnerObject::SpinnerObject(std::string name, float spin) : GameObject(name)
{
	spinnerMachine = new StateMachine();
	spinVal = spin;
}


SpinnerObject::~SpinnerObject()
{
	delete spinnerMachine;
}

void SpinnerObject::SetupStateMachine()
{

	StateFunc SpinFunc = [](void* data)
	{
		SpinnerObject* realData = (SpinnerObject*)data;
		realData->GetPhysicsObject()->SetAngularVelocity(Vector3(0, realData->spinVal, 0));

	};


	GenericState* spinState = new GenericState(SpinFunc, (void*)this);
	spinnerMachine->AddState(spinState);


}


