#pragma once
#include <functional>
#include <type_traits>
#include <utility>
#include <vector>

namespace fp {

inline constexpr auto identity = [](auto&& x) -> decltype(auto) {
    return std::forward<decltype(x)>(x);
};

template <typename F>
auto compose(F&& f) {
    return std::forward<F>(f);
}

template <typename F, typename G, typename... Rest>
auto compose(F&& f, G&& g, Rest&&... rest) {
    auto inner = compose(std::forward<G>(g), std::forward<Rest>(rest)...);
    return [f = std::forward<F>(f), inner = std::move(inner)](auto&&... xs) {
        return f(inner(std::forward<decltype(xs)>(xs)...));
    };
}

template <typename F>
auto pipe(F&& f) {
    return std::forward<F>(f);
}

template <typename F, typename G, typename... Rest>
auto pipe(F&& f, G&& g, Rest&&... rest) {
    auto tail = pipe(std::forward<G>(g), std::forward<Rest>(rest)...);
    return [f = std::forward<F>(f), tail = std::move(tail)](auto&&... xs) {
        return tail(f(std::forward<decltype(xs)>(xs)...));
    };
}

template <typename F>
auto curry(F f) {
    return [f = std::move(f)](auto x) {
        return [f, x = std::move(x)](auto y) {
            return f(x, y);
        };
    };
}

template <typename Pred>
auto filter(Pred pred) {
    return [pred = std::move(pred)](const auto& xs) {
        using T = typename std::decay_t<decltype(xs)>::value_type;
        std::vector<T> out;
        for (const auto& x : xs) {
            if (pred(x)) out.push_back(x);
        }
        return out;
    };
}

template <typename F>
auto map(F f) {
    return [f = std::move(f)](const auto& xs) {
        using T = typename std::decay_t<decltype(xs)>::value_type;
        using R = std::invoke_result_t<F, const T&>;
        std::vector<R> out;
        out.reserve(xs.size());
        for (const auto& x : xs) out.push_back(f(x));
        return out;
    };
}

template <typename Init, typename Op>
auto reduce(Init init, Op op) {
    return [init = std::move(init), op = std::move(op)](const auto& xs) {
        auto acc = init;
        for (const auto& x : xs) acc = op(acc, x);
        return acc;
    };
}

template <typename T>
auto constant(T v) {
    return [v = std::move(v)](auto&&...) { return v; };
}
}  
