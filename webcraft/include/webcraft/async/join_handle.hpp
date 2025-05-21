#pragma once
#include <webcraft/async/awaitable.hpp>
#include <async/task.h>
#include <webcraft/concepts.hpp>

namespace webcraft::async
{

	class join_handle
	{
	private:
		class event
		{
		private:
			std::vector<std::function<void()>> callbacks;
			std::atomic<bool> flag;

		public:
			event() = default;

			void arm(std::function<void()> &&callback)
			{
				callbacks.push_back(callback);
			}

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
		std::shared_ptr<event> ev;
		Task<void> task;

	public:
		join_handle(Task<void> &&t) : task(t), ev({})
		{
			auto fn = [&](Task<void> &&task) -> Task<void>
			{
				co_await task;

				ev->invoke();
			};

			this->task = fn(std::move(task));
		}

		join_handle(join_handle&& h) : task(h.task), ev(h.ev) {
		}

		inline Task<void> get_task()
		{
			return task;
		}

		inline bool await_ready() { return ev->is_set(); }
		inline void await_suspend(std::coroutine_handle<> h)
		{
			ev->arm([h]()
			{
				h.resume();
			});
		}
		inline void await_resume() {}
	};

};
