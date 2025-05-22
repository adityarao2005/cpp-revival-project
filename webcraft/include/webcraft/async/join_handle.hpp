#pragma once
#include <webcraft/async/awaitable.hpp>
#include <async/task.h>
#include <webcraft/concepts.hpp>

namespace webcraft::async
{
	class AsyncRuntime;
	class ExecutorService;

	/// @brief A class that represents a handle to a spawned task that can be joined.
	class join_handle
	{
#pragma region "friend classes"
		friend class AsyncRuntime;
		friend class ExecutorService;
#pragma endregion

	private:
		// contains a callback event which is called when the task is completed
		class event
		{
		private:
			// callbacks (ik YAGNI should be followed but when we need to reuse this we can just copy paste into a new file and include header)
			std::vector<std::function<void()>> callbacks;
			// atomic flag to check if the event is set
			std::atomic<bool> flag;

		public:
			// constructor
			event() = default;

			// adds a listener to the event
			void add_listener(std::function<void()> &&callback)
			{
				callbacks.push_back(callback);
			}

			// invokes the event and calls all the callbacks
			void invoke()
			{
				for (auto callback : callbacks)
					callback();

				bool p = false;
				flag.compare_exchange_strong(p, true, std::memory_order_relaxed);
			}

			bool is_set()
			{
				return flag;
			}
		};
		// pointer to the event, keep it as shared since when we want to move the join_handle we need to keep the event alive
		std::shared_ptr<event> ev;

	protected:
		// the task that is being awaited, keep this in protected cuz we need friends to access it
		Task<void> task;

		// basic constructor for friends only
		join_handle(Task<void> &&t) : task(t), ev({})
		{
			// create a new task which invokes the events
			auto fn = [&](Task<void> &&task) -> Task<void>
			{
				co_await task;

				ev->invoke();
			};

			this->task = fn(std::move(task));
		}

	public:
		// moveable constructor, moves the task and event
		join_handle(join_handle &&h) : task(h.task), ev(h.ev)
		{
		}

		// awaitable concept functions
		inline bool await_ready() { return ev->is_set(); }
		inline void await_suspend(std::coroutine_handle<> h) noexcept
		{
			ev->add_listener([h]()
							 { h.resume(); });
		}
		inline void await_resume() {}
	};

};
