#pragma once
#include <webcraft/async/awaitable.hpp>
#include <webcraft/concepts.hpp>

namespace webcraft::async
{
	class async_runtime;
	class executor_service;

	/// @brief A class that represents a handle to a spawned task that can be joined.
	class join_handle
	{
#pragma region "friend classes"
		friend class async_runtime;
		friend class executor_service;
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
			event() : flag(false)
			{
			}

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
		task<void> t;

		// basic constructor for friends only
		join_handle(task<void> &&t) : t(std::move(t)), ev(std::make_shared<event>())
		{

			// create a new task which invokes the events
			auto fn = [&](task<void> &&t) -> task<void>
			{
				co_await t;

				ev->invoke();
			};

			this->t = fn(std::move(this->t));
		}

	public:
		// moveable constructor, moves the task and event
		join_handle(join_handle &&h) : t(h.t), ev(h.ev)
		{
		}

		// awaitable concept functions
		bool await_ready() { return ev->is_set(); }
		void await_suspend(std::coroutine_handle<> h) noexcept
		{
			ev->add_listener([h]()
							 { h.resume(); });
		}
		constexpr void await_resume() {}
	};

};
