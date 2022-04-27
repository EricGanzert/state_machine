#include "state_machine.h"

using namespace std;

StateMachine::StateMachine() :
	m_eventQueueThread(thread(&StateMachine::eventQueueProc, this))
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
	event.finished = finished;
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
	m_eventQueue.clear();
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
	if (m_state != Idle)
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

}

//void queueEvent(const Event& type);
//void handleEvent(const Event& event);
//bool handleEventWhileIdle(const Event& event);
//bool handleEventWhileActive(const Event& event);