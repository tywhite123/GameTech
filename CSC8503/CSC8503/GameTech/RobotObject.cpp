#include "RobotObject.h"
#include "../CSC8503Common/State.h"
#include "../CSC8503Common/Debug.h"


RobotObject::RobotObject(int objID, std::string name):GameObject(objID, name)
{
	robotMachine = new StateMachine();
	iter = 0;
	pathFound = false;
}

RobotObject::~RobotObject()
{
	delete robotMachine;
}

void RobotObject::SetupStateMachine()
{

	StateFunc FindFunc = [](void *data)
	{
		
		RobotObject* robot = (RobotObject*)data;
		robot->path.clear();
		NavigationPath outPath;

		Vector3 start(robot->GetTransform().GetWorldPosition);
		Vector3 end(robot->player->GetTransform().GetWorldPosition);

		robot->grid->FindPath(start, end, outPath);

		Vector3 pos;
		while (outPath.PopWaypoint(pos))
			robot->path.push_back(pos);

		robot->pathFound = true;
	};


	StateFunc ChaseFunc = [](void* data) {
		RobotObject* robot = (RobotObject*)data;
		for (int i = 0; i < robot->path.size(); ++i) {
			Vector3 a = robot->path[i - 1] * 2;
			Vector3 b = robot->path[i] * 2;

			//TODO: follow path;

			//NCL::CSC8503::Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
		}
	};

	GenericState* findState = new GenericState(FindFunc, (void*)this);
	robotMachine->AddState(findState)


}

void RobotObject::OnCollisionBegin(GameObject * otherObject)
{
}


