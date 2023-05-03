#include "StateMachine.hpp"

namespace Sonar
{
	void StateMachine::AddState(State* newState, bool isReplacing)
	{
		this->_isAdding = true;
		this->_isReplacing = isReplacing;

		this->_newState = newState;
	}

	void StateMachine::RemoveState()
	{
		this->_isRemoving = true;
	}

	void StateMachine::ProcessStateChanges()
	{
		if (this->_isRemoving && !this->_states.empty())
		{
			State* current = this->_states.top();
			this->_states.pop();
			if(current != nullptr)
				delete current;

			if (!this->_states.empty())
			{
				this->_states.top()->Resume();
			}

			this->_isRemoving = false;
		}

		if (this->_isAdding)
		{
			if (!this->_states.empty())
			{
				if (this->_isReplacing)
				{
					State* current = this->_states.top();
					this->_states.pop();
					if (current != nullptr)
						delete current;
				}
				else
				{
					this->_states.top()->Pause();
				}
			}

			this->_states.push(this->_newState);
			this->_states.top()->Init();
			this->_isAdding = false;
		}
	}

	State* &StateMachine::GetActiveState()
	{
		return this->_states.top();
	}
}