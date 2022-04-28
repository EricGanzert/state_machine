#include "state_machine.h"

#include <iostream>

using namespace std;

StateMachine::StateMachine(const Config& config)
	: m_config(config)
	, m_eventQueueThread(thread(&StateMachine::eventQueueProc, this))
{
	
}

StateMachine::~StateMachine()
{
	if (m_state != State::Disabled)
	{
		stop();
	}
}

void StateMachine::start(const Config& config)
{
	if (!(m_state == State::Startup || m_state == State::Disabled))
	{
		return;
	}

	m_state = State::Startup;
	promise<void> finished;

	Event event{};
	event.type = EventType::Enable;
	event.data.config = config;
	event.finished = &finished;
	queueEvent(event);

	finished.get_future().get();
}

void StateMachine::stop()
{
	if (m_state == State::Disabled)
	{
		return;
	}

	Event event{};
	event.type = EventType::Disable;
	queueEvent(event);

	if (m_eventQueueThread.joinable())
	{
		m_eventQueueThread.join();
	}

	lock_guard<mutex> _(m_queueMutex);
	queue<Event> temp;
	m_eventQueue.swap(temp);
}

void StateMachine::queueStartEvent(const Config& config)
{
	if (!(m_state == State::Startup || m_state == State::Disabled))
	{
		return;
	}

	m_state = State::Startup;

	Event event{};
	event.type = EventType::Enable;
	event.data.config = config;
	queueEvent(event);
}

void StateMachine::queueIdleEvent()
{
	if (!(m_state == State::Startup || m_state == State::Running))
	{
		return;
	}

	Event event{};
	event.type = EventType::Idle;
	queueEvent(event);
}

void StateMachine::queueRunEvent(Speed speed)
{
	if (m_state != State::Idling)
	{
		return;
	}

	Event event{};
	event.type = EventType::Run;
	event.data.speed = speed;
	queueEvent(event);
}

void StateMachine::queueDisableEvent()
{
	if (m_state == State::Disabled)
	{
		return;
	}

	Event event{};
	event.type = EventType::Disable;
	queueEvent(event);
}

void StateMachine::eventQueueProc()
{
	while (m_state != State::Disabled)
	{
		Event event{};
		{
			unique_lock<mutex> lk(m_queueMutex);
			m_itemQueued.wait(
				lk, [&] { return !m_eventQueue.empty(); });

			event = move(m_eventQueue.front());
			m_eventQueue.pop();
		}
		handleEvent(event);
	}
}

void StateMachine::queueEvent(const Event& event)
{
	{
		lock_guard<mutex> _(m_queueMutex);
		m_eventQueue.emplace(event);
	}
	m_itemQueued.notify_all();
}

void StateMachine::handleEvent(const Event& event)
{
	// events handled in all states
	if (m_state == State::Disabled)
	{
		return;
	}
	
	switch (m_state)
	{
	case State::Running:
		handleEventWhileActive(event);
		break;
	case State::Idling:
		handleEventWhileIdle(event);
		break;
	case State::Startup:
		handleEventInStartup(event);
		break;
	case State::Disabled:
		if (event.type == EventType::Enable)
		{
			enable();
		}
		break;
	}
}

void StateMachine::enable()
{
	cout << "doing the work of enabling this state machine" << endl;
	m_state = State::Startup;
}

void StateMachine::disable()
{
	cout << "doing the work of disabling this state machine" << endl;
	m_state = State::Disabled;
}

void StateMachine::handleEventWhileIdle(const Event& event)
{
	if (event.type == EventType::Run)
	{
		cout << "Beginning to run at speed " << event.data.speed << endl;
		m_state = State::Running;
		return;
	}

	if (event.type == EventType::Disable)
	{
		cout << "Cleaning up idle state for a shutdown" << endl;
		m_state = State::Disabled;
		return;
	}
}

void StateMachine::handleEventWhileActive(const Event& event)
{
	if (event.type == EventType::Run)
	{
		cout << "Continuing to run, now at speed " << event.data.speed << endl;
		return;
	}

	if (event.type == EventType::Disable)
	{
		cout << "Cleaning up running state for a shutdown" << endl;
		m_state = State::Disabled;
		return;
	}

	if (event.type == EventType::Idle)
	{
		cout << "Stopping running and beginning to idle" << endl;
		m_state = State::Idling;
		return;
	}
}

void StateMachine::handleEventInStartup(const Event& event)
{
	if (event.type == EventType::Enable)
	{
		cout << "Starting up state machine, beginning to idle" << endl;
		m_config = event.data.config;
		m_state = State::Idling;
	}

	if (event.type == EventType::Disable)
	{
		cout << "Shutting down state machine" << endl;
		m_state = State::Disabled;
	}
}