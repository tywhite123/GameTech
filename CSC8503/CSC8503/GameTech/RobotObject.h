#pragma once
#include "../CSC8503Common/GameObject.h"
#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/PlayerObject.h"
#include "../CSC8503Common/NavigationGrid.h"

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
	NavigationGrid* grid;
	PlayerObject* player;
	vector<Vector3> path;

	Vector3 currentNode;
	Vector3 toNode;

	bool pathFound;
	int iter;
};

