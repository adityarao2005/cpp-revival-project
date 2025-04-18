#pragma once

#include <memory>
#include <string>

namespace webcraft
{
    class ApplicationConfig;

    /// @brief The ApplicationContextScope class is used to manage the different scopes in the application context.
    /// There are different scopes in the application context, such as the global scope and prototype scope.
    class ApplicationContextScope
    {
    protected:
        ApplicationContext &context_;

        /// @brief Gets the object of the given type and name from the scope.
        /// @param type the type of the object
        /// @param name the name of the object
        /// @return the shared pointer to the object
        virtual std::shared_ptr<void> get(const std::type_info &type, const std::string &name) = 0;

        /// @brief Gets the object of the given type from the scope.
        /// @param type the type of the object
        /// @return the shared pointer to the object
        virtual std::shared_ptr<void> get(const std::type_info &type) = 0;

        /// @brief Sets the object of the given type and name in the scope.
        /// @param type the type of the object
        /// @param name the name of the object
        /// @param object the shared pointer to the object
        /// @details If the object is already present in the context scope, it will be replaced.
        virtual void set(const std::type_info &type, const std::string &name, std::shared_ptr<void> object) = 0;

        /// @brief Sets the object of the given type in the scope.
        /// @param type the type of the object
        /// @param object the shared pointer to the object
        virtual void set(const std::type_info &type, std::shared_ptr<void> object) = 0;

        /// @brief Removes the object of the given type and name from the scope.
        /// @param type the type of the object
        /// @param name the name of the object
        virtual void remove(const std::type_info &type, const std::string &name) = 0;

        /// @brief Removes the object of the given type from the scope.
        /// @param type the type of the object
        virtual void remove(const std::type_info &type) = 0;

    public:
        ApplicationContextScope(ApplicationContext &context) : context_(context)
        {
            // Initialize the context
        }

        ~ApplicationContextScope()
        {
            clear();
        }

        /// @brief Gets the object of the given type and name from the scope.
        /// @tparam T the type of the object
        /// @return the shared pointer to the object
        template <typename T>
        std::shared_ptr<T> get()
        {
            auto ptr = get(typeid(T));
            if (ptr)
            {
                return std::static_pointer_cast<T>(ptr);
            }
            return nullptr;
        }

        /// @brief Gets the object of the given type and name from the scope.
        /// @tparam T the type of the object
        /// @param name the name of the object
        /// @return the shared pointer to the object
        template <typename T>
        std::shared_ptr<T> get(const std::string &name)
        {
            auto ptr = get(typeid(T), name);
            if (ptr)
            {
                return std::static_pointer_cast<T>(ptr);
            }
            return nullptr;
        }

        /// @brief sets the object of the given type and name in the scope.
        /// @tparam T the type of the object
        /// @param object the shared pointer to the object
        template <typename T>
        void set(std::shared_ptr<T> object)
        {
            auto type = typeid(T);
            std::shared_ptr<void> ptr = std::static_pointer_cast<void>(object);

            set(type, ptr);
        }

        /// @brief sets the object of the given type and name in the scope.
        /// @tparam T the type of the object
        /// @param name the name of the object
        /// @param object the shared pointer to the object
        template <typename T>
        void set(const std::string &name, std::shared_ptr<T> object)
        {
            auto type = typeid(T);
            std::shared_ptr<void> ptr = std::static_pointer_cast<void>(object);

            set(type, name, ptr);
        }

        /// @brief Removes the object of the given type from the scope.
        /// @tparam T the type of the object
        template <typename T>
        void remove()
        {
            auto type = typeid(T);
            remove(type);
        }

        /// @brief Removes the object of the given type and name from the scope.
        /// @tparam T the type of the object
        /// @param name the name of the object
        template <typename T>
        void remove(const std::string &name)
        {
            auto type = typeid(T);
            remove(type, name);
        }

        /// @brief Clears the scope.
        virtual void clear() = 0;
    };

    /// @brief Handle for the creation of scopes in the application context.
    /// @details The ScopeGuard class is used to create and destroy scopes in the application context.
    /// It is used to manage the lifetime of the scope and ensure that it is destroyed when it goes out of scope.
    class ScopeGuard
    {
    private:
        ApplicationContext &context;
        std::string name;

    public:
        /// @brief Constructor for the ScopeGuard class.
        /// @param context the context of the application
        /// @param name the name of the scope
        ScopeGuard(ApplicationContext &context, const std::string &name) : context(context), name(name)
        {
            // Initialize the scope guard
            context.create_scope(name);
        }

        /// @brief Destructor for the ScopeGuard class.
        ~ScopeGuard()
        {
            // Destroy the scope guard
            context.destroy_scope(name);
        }
    };
    /// @brief The ApplicationContext class is used to manage the application context.
    /// @details The ApplicationContext class is used to manage the application state, events, and other framework related tasks.
    /// It is passed to the run method of the Application class.
    class ApplicationContext
    {
    private:
        void create_scope(std::string name);
        void destroy_scope(std::string name);

    public:
        friend class ScopeGuard;

        void init(ApplicationConfig &config);
    };
}