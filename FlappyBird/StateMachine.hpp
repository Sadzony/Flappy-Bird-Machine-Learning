#pragma once

#include <memory>
#include <stack>

#include "State.hpp"

namespace Sonar
{
	typedef std::unique_ptr<State> StateRef;

	class StateMachine
	{
	public:
		StateMachine() { }
		~StateMachine() { }

		void AddState(State* newState, bool isReplacing = true);
		void RemoveState();
		// Run at start of each loop in Game.cpp
		void ProcessStateChanges();

		State* &GetActiveState();

	private:
		std::stack<State*> _states;
		State* _newState;

		bool _isRemoving;
		bool _isAdding, _isReplacing;
	};
}