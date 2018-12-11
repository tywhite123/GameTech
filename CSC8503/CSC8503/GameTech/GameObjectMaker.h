#pragma once
#include "../CSC8503Common/GameObject.h"

using namespace NCL::CSC8503;

class GameObjectMaker
{
public:
	GameObjectMaker();
	~GameObjectMaker();

	static GameObject* AddPlayerToWorld(int objID, const Vector3& position, float radius, float inverseMass = 10.0f);
	static GameObject* AddWallToWorld(int objID, const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
	static GameObject* AddGoalToWorld(int objID, const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
	static GameObject* AddMovingToWorld(int objID, const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
	static GameObject* AddRobotToWorld(int objID, const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
	static GameObject* AddFloorToWorld(int objID, const Vector3& position);

};

