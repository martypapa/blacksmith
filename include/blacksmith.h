#ifndef MPAPA_BLACKSMITH
#define MPAPA_BLACKSMITH

#include <functional>
#include <memory>
#include <type_traits>

namespace blacksmith {

template <class T>
struct as_function : public as_function<decltype(&T::operator())>
{};

template <typename T>
struct is_shared_ptr : std::false_type
{};

template <typename T>
struct is_shared_ptr<std::shared_ptr<T>> : std::true_type
{};

template <typename T>
struct is_unique_ptr : std::false_type
{};
template <typename T>
struct is_unique_ptr<std::unique_ptr<T>> : std::true_type
{};

template <typename T>
struct remove_ptr
{
    using type = T;
};
template <typename T>
struct remove_ptr<std::shared_ptr<T>>
{
    using type = T;
};
template <typename T>
struct remove_ptr<std::unique_ptr<T>>
{
    using type = T;
};
template <typename T>
using remove_ptr_t = typename remove_ptr<T>::type;

template <class Ret, class Arg, class... Rest>
struct as_function<Ret(Arg, Rest...)>
{
    using type = std::function<Ret(Arg, Rest...)>;
    using return_type = Ret;
    using first_arg = Arg;
};

template <class Ret, class Arg, class... Rest>
struct as_function<Ret (*)(Arg, Rest...)>
{
    using type = std::function<Ret(Arg, Rest...)>;
    using return_type = Ret;
    using first_arg = Arg;
};

template <class C, class Ret, class Arg, class... Rest>
struct as_function<Ret (C::*)(Arg, Rest...) const>
{
    using type = std::function<Ret(Arg, Rest...)>;
    using return_type = Ret;
    using first_arg = Arg;
};

template <typename F>
using first_arg = typename as_function<F>::first_arg;

//! Modify the existing \a value, using the function \a f
//! \param f A function of type <void(T*)> or <void(T&)>, that modifies \a value
//! \param value The value to be modified
template <typename T, typename F>
constexpr T *build_on(T *value, const F &f)
{
    if constexpr (std::is_pointer<first_arg<F>>::value) {
        f(value);
    } else {
        static_assert(std::is_reference<first_arg<F>>::value, "requires a reference");
        f(*value);
    }
    return value;
}

//! Modify the existing \a value, using the function \a f
//! \param f A function of type <void(T&)>, that modifies \a value
//! \param value The value to be modified
template <typename T, typename F>
constexpr T &build_on(T &value, const F &f)
{
    static_assert(std::is_reference<first_arg<F>>::value, "requires a reference");
    f(value);
    return value;
}

//! Construct and modify a new class of type <T> using the function, \a f
//! \param f A function of type <void(T&)>, that modifies the constructed value
//! \param args... Arguments used to construct the value
template <typename F, typename... Args>
constexpr auto build(const F &f, Args &&... args) -> std::remove_reference_t<first_arg<F>>
{
    static_assert(std::is_reference<first_arg<F>>::value, "requires a reference");
    using T = std::remove_reference_t<first_arg<F>>;
    T x(std::forward<Args>(args)...);
    f(x);
    return x;
}

//! Construct and modify a new class of type <T> using the function, \a f
//! \param f A function of type <void(T&)> or <void(T*)>, that modifies the constructed value
//! \param args... Arguments used to construct the value
template <typename F, typename... Args>
constexpr auto build_new(const F &f, Args &&... args)
    -> std::remove_pointer_t<std::remove_cv_t<std::remove_reference_t<first_arg<F>>>> *
{
    using T = std::remove_pointer_t<std::remove_cv_t<std::remove_reference_t<first_arg<F>>>>;
    auto x = new T(std::forward<Args>(args)...);
    if constexpr (std::is_pointer<first_arg<F>>::value) {
        f(x);
    } else {
        f(*x);
    }
    return x;
}

//! Construct and modify a unique_ptr<T> using the function, \a f
//! \param f A function of type <void(T&)> or <void(const std::unique_ptr<T>&)>, that modifies the constructed value
//! \param args... Arguments used to construct the value
template <typename F, typename... Args>
constexpr auto build_unique(const F &f, Args &&... args)
    -> std::unique_ptr<remove_ptr_t<std::remove_cv_t<std::remove_reference_t<first_arg<F>>>>>
{
    using T = typename std::remove_cv_t<std::remove_reference_t<first_arg<F>>>;
    auto x = std::make_unique<remove_ptr_t<T>>(std::forward<Args>(args)...);
    if constexpr (is_unique_ptr<T>::value) {
        f(x);
    } else {
        f(*x);
    }
    return x;
}
//! Construct and update a shared_ptr of <T> using the function, \a f
//! \param f A function of type <void(T&)> or <void(const std::shared_ptr<T>&)>, that modifies the constructed value
//! \param args... Arguments used to construct the value
template <typename F, typename... Args>
constexpr auto build_shared(const F &f, Args &&... args)
    -> std::shared_ptr<remove_ptr_t<std::remove_cv_t<std::remove_reference_t<first_arg<F>>>>>
{
    using T = typename std::remove_cv_t<std::remove_reference_t<first_arg<F>>>;
    auto x = std::make_shared<remove_ptr_t<T>>(std::forward<Args>(args)...);
    if constexpr (is_shared_ptr<T>::value) {
        f(x);
    } else {
        f(*x);
    }
    return x;
}
}  // namespace blacksmith
#endif