#include "MovingWallObject.h"
#include "../CSC8503Common/StateTransition.h"


MovingWallObject::MovingWallObject(int objID, string name) : GameObject(objID, name)
{
	moveMachine = new StateMachine();
	
	

}

MovingWallObject::~MovingWallObject()
{
	delete moveMachine;
}

void MovingWallObject::SetupStateMachine()
{

	z = transform.GetWorldPosition().z;
	x = transform.GetWorldPosition().x;
	y = transform.GetWorldPosition().y;

	//std::cout << *data << std::endl;

	//std::cout << transform.GetWorldPosition() << std::endl;

	StateFunc AFunc = [](void*data)
	{
		MovingWallObject* realData = (MovingWallObject*)data;
		Vector3 worldPos = realData->transform.GetWorldPosition();
		realData->z -= 0.5f;
		realData->transform.SetWorldPosition(Vector3(realData->x, realData->y, realData->z));
		//std::cout << "In State A! " << worldPos << " " << (&realData->transform.GetWorldPosition())->z << std::endl;
	};

	StateFunc BFunc = [](void*data)
	{
		MovingWallObject* realData = (MovingWallObject*)data;
		Vector3 worldPos = realData->transform.GetWorldPosition();
		realData->z += 0.5f;
		realData->transform.SetWorldPosition(Vector3(realData->x, realData->y, realData->z));
		//std::cout << "	In State B! " << worldPos << " " << (&realData->transform.GetWorldPosition())->z << std::endl;

	};


	GenericState* stateA = new GenericState(AFunc, (void*)this);
	GenericState* stateB = new GenericState(BFunc, (void*)this);
	moveMachine->AddState(stateA);
	moveMachine->AddState(stateB);


	GenericTransition<float&, float>* transitionA = new GenericTransition<float&, float>(
		GenericTransition<float&, float>::GreaterThanTransition, z /*(&this->transform.GetWorldPosition())->z*/, 159.0f, stateB, stateA);
		

	GenericTransition<float&, float>* transitionB = new GenericTransition<float&, float>(
		GenericTransition<float&, float>::LessThanTransition, z /*(&this->transform.GetWorldPosition())->z*/, 121.0f, stateA, stateB);

	moveMachine->AddTransition(transitionA);
	moveMachine->AddTransition(transitionB);
}


