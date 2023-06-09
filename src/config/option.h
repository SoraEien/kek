/**
 * src/config/option.h
 *
 * Copyright (c) 2021-2023 Bartek Kryza <bkryza@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <string>
#include <utility>

namespace clanguml {
namespace config {

template <typename T> void append_value(T &l, const T &r) { l = r; }

enum class option_inherit_mode { kOverride, kAppend };

template <typename T> struct option {
    option(std::string name_,
        option_inherit_mode im = option_inherit_mode::kOverride)
        : name{std::move(name_)}
        , value{}
        , inheritance_mode{im}
    {
    }
    option(std::string name_, T initial_value,
        option_inherit_mode im = option_inherit_mode::kOverride)
        : name{std::move(name_)}
        , value{std::move(initial_value)}
        , has_value{true}
        , inheritance_mode{im}
    {
    }

    void set(const T &v)
    {
        value = v;
        is_declared = true;
        has_value = true;
    }

    void override(const option<T> &o)
    {
        if (o.is_declared && inheritance_mode == option_inherit_mode::kAppend) {
            append_value(value, o.value);
            is_declared = true;
            has_value = true;
        }
        else if (!is_declared && o.is_declared) {
            value = o.value;
            is_declared = true;
            has_value = true;
        }
    }

    void operator()(const T &v) { set(v); }

    T &operator()() { return value; }

    const T &operator()() const { return value; }

    operator bool() const { return has_value; }

    std::string name;
    T value;
    bool is_declared{false};
    bool has_value{false};
    option_inherit_mode inheritance_mode;
};
} // namespace config
} // namespace clanguml
