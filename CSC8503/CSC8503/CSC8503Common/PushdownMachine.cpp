#include "PushdownMachine.h"
#include "PushdownState.h"
using namespace NCL::CSC8503;

PushdownMachine::PushdownMachine()
{
	activeState = nullptr;
}

PushdownMachine::~PushdownMachine()
{
}

void PushdownMachine::AddState(PushdownState * s)
{
	stateStack.push(s);
	activeState = stateStack.top();
}


void PushdownMachine::Update() {
	activeState->PushdownUpdate(&activeState);
}