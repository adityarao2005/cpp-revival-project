#include <thread>
#include <chrono>
#include <webcraft/webcraft.hpp>

webcraft::async::task<void> example_task()
{
    co_return;
}

int main()
{
    webcraft::async::async_runtime &runtime = webcraft::async::async_runtime::get_instance();

    runtime.run_async(example_task);
}