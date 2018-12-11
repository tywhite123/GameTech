#pragma once
#include <stack>
#include "StateTransition.h"
#include <map>


enum StateType
{
	Main_Menu,
	Lobby,
	Game,
	Pause
};

namespace NCL {
	namespace CSC8503 {
		class PushdownState;

		typedef std::multimap<State*, StateTransition*> TransitionContainer;
		typedef TransitionContainer::iterator TransitionIterator;
		typedef std::map<StateType, PushdownState*> StateContainer;

		class PushdownMachine
		{
		public:
			PushdownMachine();
			~PushdownMachine();

			void AddState(PushdownState* s, StateType type);
			void AddTransition(StateTransition* s);

			void Update();

		protected:
			PushdownState * activeState;

			std::stack<PushdownState*> stateStack;

			StateContainer allStates;

			TransitionContainer allPushdownTransitions;

			

		};
	}
}

