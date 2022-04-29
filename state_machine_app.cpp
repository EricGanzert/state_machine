#include <state_machine.h>

#include <iostream>

using namespace std;

int main()
{
	cout << "1. Constructing and destructing a default StateMachine" << endl;
	{
		StateMachine sm;
	}
	
	cout << "2. Enabling a StateMachine blocking function" << endl;
	{
		StateMachine sm;
		sm.start(StateMachine::Config{});
	}

	cout << "3. Enabling a StateMachine asynchronously" << endl;
	{
		StateMachine sm;
		sm.queueStartEvent(StateMachine::Config{});
	}

	cout << "4. Running an idle state machine" << endl;
	{
		constexpr StateMachine::Speed TestSpeed = 10;

		StateMachine sm;
		sm.start(StateMachine::Config{});
		sm.queueRunEvent(TestSpeed);
	}

	cout << "5. Idling a running state machine" << endl;
	{
		constexpr StateMachine::Speed TestSpeed = 10;

		StateMachine sm;
		sm.start(StateMachine::Config{});
		sm.queueRunEvent(TestSpeed);
		sm.queueIdleEvent();
		this_thread::sleep_for(50ms);
	}
}