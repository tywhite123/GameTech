#include "PlayerObject.h"
#include "Debug.h"


PlayerObject::PlayerObject(string name) : GameObject(name)
{
}

PlayerObject::~PlayerObject()
{
}

void PlayerObject::OnCollisionBegin(GameObject * otherObject)
{
	if (otherObject->GetName() == "Goal") {
		level->loadNext = true;
		level->SetLevel(level->GetLevel()+1);
	}
}
