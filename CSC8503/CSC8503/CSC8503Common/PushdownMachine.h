#pragma once
#include <stack>
#include "StateTransition.h"

namespace NCL {
	namespace CSC8503 {
		class PushdownState;

		class PushdownMachine
		{
		public:
			PushdownMachine();
			~PushdownMachine();

			void AddState(PushdownState* s);
			void AddTransition(StateTransition* s);

			void Update();

		protected:
			PushdownState * activeState;

			std::stack<PushdownState*> stateStack;
		};
	}
}

