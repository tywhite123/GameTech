#include "PushdownMachine.h"
#include "PushdownState.h"
#include <string>
using namespace NCL::CSC8503;

PushdownMachine::PushdownMachine()
{
	activeState = nullptr;
}

PushdownMachine::~PushdownMachine()
{
}

void PushdownMachine::AddState(PushdownState * s, StateType type)
{
	allStates.insert(std::pair<StateType, PushdownState*>(type, s));
	if (activeState == nullptr) {
		activeState = s;
		stateStack.push(s);
	}
	/*stateStack.push(s);
	activeState = stateStack.top();*/
}


void PushdownMachine::Update() {
	PushdownState* s;
	PushdownState::PushdownResult result = activeState->PushdownUpdate(&s);

	switch(result)
	{
	case PushdownState::Pop:
		stateStack.pop();
		activeState = stateStack.top();
		break;
	case PushdownState::Push:
		//AddState(s);
		break;
	case PushdownState::NoChange:
		break;
	}
}