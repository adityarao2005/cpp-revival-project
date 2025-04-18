#include <fmt/core.h>
#include "webcraft/app.hpp"
#include <coroutine>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

class Application : public webcraft::Application
{
public:
    using super = webcraft::Application;

    Application()
    {
        fmt::print("Application created.\n");
    }

    webcraft::ApplicationConfig init(int argc, char **argv) override
    {
        fmt::print("Initializing application...\n");
        webcraft::ApplicationConfig config = super::init(argc, argv);
        // Initialize the application configuration here

        return config;
    }

    void run(webcraft::ApplicationContext &context) override
    {
        fmt::print("Running application...\n");

        // Simulate some work
        for (int i = 0; i < 5; ++i)
        {
            fmt::print("Working... {}/5\n", i + 1);
            std::this_thread::sleep_for(1s);
        }

        fmt::print("Application finished.\n");
        // You can use the context to manage application state, events, etc.
    }
};

RUN_APP(Application)
