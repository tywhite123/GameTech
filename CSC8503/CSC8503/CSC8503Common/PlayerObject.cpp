#include "PlayerObject.h"
#include "GameWorld.h"
#include "Debug.h"


PlayerObject::PlayerObject(string name) : GameObject(name)
{
}

PlayerObject::~PlayerObject()
{
}

void PlayerObject::OnCollisionBegin(GameObject * otherObject)
{
	if(otherObject->GetName() == "Goal")
	{
		NCL::Debug::Print("Level Finished", Vector2(1280/3, 720/2), Vector4(1,1,1,1));
		std::cout << "Level finished" << std::endl;

	}
}
