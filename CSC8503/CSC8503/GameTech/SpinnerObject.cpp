#include "SpinnerObject.h"



SpinnerObject::SpinnerObject(std::string name) : GameObject(name)
{
	spinnerMachine = new StateMachine();
}


SpinnerObject::~SpinnerObject()
{
	delete spinnerMachine;
}
