# WebCraft

## Features / Modules

- Context & Dependency Injection and Application Builder framework
- Asynchronous Runtime and Executors (IO bound - not public.. mostly internal and CPU bound executors - public) - works if enabled
- Basic networking API (Sockets, Server Sockets, async read & write)
- Web API and WebApplication wrapper (everything before was mostly console or cmdline based)
- Raw request/response handling with streams
- DataTransferObject API and serialization
- potentially support of creating adapters for other servers
- eventually have TLS support, browser rendering, etc


### Context & Dependency Injection and Application Builder

The base application class is `webcraft::Application` and it contains the following members:

```cpp
class Application
{
public:
    virtual void run(ApplicationContext &context) = 0;

    virtual ApplicationConfig init(int argc, char **argv);

    virtual ~Application() = default;

    static void run(Application *app, int argc, char **argv);
};
```
The application requires all users of the framework to override the `webcraft::Application::run(ApplicationContext&)` method since that's where most of the work in the application will occur.

You can optionally override the `webcraft::Application::init(int argc, char** argv)` method (its recommended that you do to specify your own configurations - we'll discuss more about that soon). The base implementation will allow users to access the command line arguments from the global scope from the ApplicationContext.

An example application using this interface is below:
```cpp
#include <fmt/core.h>
#include "webcraft/app.hpp"
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

        return config;
    }

    void run(webcraft::ApplicationContext &context) override
    {
        fmt::print("Running application...\n");

        for (int i = 0; i < 5; ++i)
        {
            fmt::print("Working... {}/5\n", i + 1);
            std::this_thread::sleep_for(1s);
        }

        fmt::print("Application finished.\n");
    }
};

RUN_APP(Application)
```

Here we've built a macro which will create the main method for you and run the application (seems like a lot of writing for a C++ program).

The reason for this is to allow clients to use our dependency injection framework through ApplicationContext.
The way that this will work is below:
- ApplicationContext is meant to be a thread-safe context for all those who use it and it will hopefully replace the need for using Singletons when using this framework (can't completely remove it).
- You can add global-scope like objects in the context (during configuration or while running the application)
- You can create prototype-scope like objects in the context which will create a new instance of the objects when called (during configuration or while running the application)
- You can create your own scopes which will be bound by a certain lifetime and add objects to that scope so that they can be injected later in the scope (this definetly while running the application) and this will be bound by a `ScopeGuard`. You would do so as shown below:
```cpp
async::task<void> parse_http_request(http::request req, http::response resp)
{
    ScopeGuard guard(ctx, "request"); // scope created here

    ctx.scoped("request").put(req);
    ctx.scoped("request").put(resp);

    std::unique_ptr<User> user = await parse_user(req.session);
    ctx.scoped("request").put("user", user);

    invoke_request_handler_chain(); // call middlewares and request handler
} // scope destroyed here via RAII

// Later ... potentially .. not how I'm gonna design it to say
ROUTE("/", async::task<std::string>, printUser, User, user, {
    std::cout << "User id is:" << user->id() << std::endl;
    co_return user->id();
})

// which could expand to
auto printUser = [] (ApplicationContext& ctx) -> async::task<std::string> {
    std::shared_ptr<User> user = ctx.get<User>();
    std::cout << "User id is:" << user->id() << std::endl;
    co_return std::string;
}
routes.add("/", printUser);
``` 

- The scopes should be able to stay consistent even in a multithreaded environment even if the functions need to be suspended in one thread and resumed in another

