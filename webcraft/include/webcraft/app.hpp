#pragma once

#include <memory>
#include <functional>
#include "context.hpp"

namespace webcraft
{

    class Application;
    class ApplicationContext;
    class ApplicationConfig;

    /// @brief The Application class is the base class for all applications in the webcraft framework.
    /// It provides a way to initialize and run the application.
    /// @details The Application class is an abstract class that defines the interface for all applications.
    class Application
    {
    public:
        /// @brief Abstract run method that must be implemented by the derived class.
        /// @param context The context of the given application. Can be used for dependency injection and other framework related tasks.
        virtual void run(ApplicationContext &context) = 0;

        /// @brief Initializes the application and returns the application configuration.
        /// @param argc the number of command line arguments
        /// @param argv the command line arguments
        /// @return configuration
        virtual ApplicationConfig init(int argc, char **argv);

        /// @brief Destructor for the Application class.
        virtual ~Application() = default;

        /// @brief Static method to run the application.
        /// @param app The application instance to run.
        static void run(Application *app, int argc, char **argv);
    };

    /// @brief The ApplicationConfig class is used to store the configuration of the application.
    class ApplicationConfig
    {
    };
}

/// @brief Macro to define the main function for the application.
/// @param CLASS_NAME The name of the application class that inherits from webcraft::Application.
#define RUN_APP(CLASS_NAME)                           \
    int main(int argc, char **argv)                   \
    {                                                 \
        CLASS_NAME app;                               \
        webcraft::Application::run(&app, argc, argv); \
        return 0;                                     \
    }
