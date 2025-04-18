#include "webcraft/app.hpp"

void webcraft::Application::run(Application *app, int argc, char **argv)
{
    ApplicationConfig config = app->init(argc, argv);

    ApplicationContext context;
    context.init(config);

    app->run(context);
}

webcraft::ApplicationConfig webcraft::Application::init(int argc, char **argv)
{
    ApplicationConfig config;
    // Initialize the application configuration here
    return config;
}

void webcraft::ApplicationContext::init(ApplicationConfig &config)
{
}