#pragma once
#include <functional>
#include <string>
#include <type_traits>

using LogSink = std::function<void(const std::string&)>;

template <typename Function>
auto measureExecutionTime(const std::string& label,
                          Function func,
                          bool enabled,
                          const LogSink& sink) -> std::invoke_result_t<Function>;

#include "helper.tpp"
