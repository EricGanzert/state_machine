#pragma once

#include <future>
#include <mutex>
#include <queue>
#include <thread>

class StateMachine
{
public:
	StateMachine();
	~StateMachine();

	// each state performs a narrowly defined task
	enum class State
	{
		Startup,
		Idling,
		Running,
		Disabled
	};

	struct Config {
		bool settingA{};
		bool settingB{};
	};

	using Speed = uint32_t;

	void start(const Config& config);
	void stop();

	// queue event functions
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

	void eventQueueProc();
	void queueEvent(const Event& type);
	void handleEvent(const Event& event);
	bool handleEventWhileIdle(const Event& event);
	bool handleEventWhileActive(const Event& event);

	std::atomic<State> m_state{ State::Startup };
	std::atomic<Config> m_config{};

	std::mutex m_queueMutex;
	std::queue<EventType> m_eventQueue;
	std::condition_variable m_itemQueued;

	std::thread m_eventQueueThread;
};