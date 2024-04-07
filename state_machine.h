#pragma once

#include <future>
#include <mutex>
#include <queue>
#include <thread>

class StateMachine
{
public:
	struct Config {
		bool settingA = false;
		bool settingB = false;
	};

	StateMachine();
	StateMachine(const Config& config);
	~StateMachine();

	// each state performs a narrowly defined task
	enum class State
	{
		Startup,
		Idling,
		Running,
		Disabled
	};

	using Speed = uint32_t;

	void start(const Config& config);
	void stop();

	// queue event functions
	void queueStartEvent(const Config& config);
	void queueIdleEvent();
	void queueRunEvent(Speed speed);
	void queueDisableEvent();

	State getState() const
	{
		return m_state.load();
	}

private:
	// Events cause a change in state
	enum class EventType
	{
		Enable,
		Run,
		Idle,
		Disable
	};

	struct Event {
		EventType type;
		union
		{
			Config config;
			Speed speed;
		} data;

		std::promise<void>* finished = nullptr;
	};

	void enable();
	void disable();

	void eventQueueProc();
	void queueEvent(const Event& event);
	void handleEvent(const Event& event);
	void handleEventWhileIdle(const Event& event);
	void handleEventWhileActive(const Event& event);
	void handleEventInStartup(const Event& event);

	std::atomic<State> m_state{ State::Startup };
	std::atomic<Config> m_config{};

	std::mutex m_queueMutex;
	std::queue<Event> m_eventQueue;
	std::condition_variable m_itemQueued;

	std::thread m_eventQueueThread;
};
