#include "RobotObject.h"
#include "../CSC8503Common/State.h"


RobotObject::RobotObject(int objID, std::string name):GameObject(objID, name)
{
	robotMachine = new StateMachine();
}

RobotObject::~RobotObject()
{
	delete robotMachine;
}

void RobotObject::SetupStateMachine()
{

	StateFunc FindFunc = [](void *data)
	{
		//RobotObject* robot = (RobotObject*)data;

	};


}

void RobotObject::OnCollisionBegin(GameObject * otherObject)
{
}


