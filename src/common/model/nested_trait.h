/**
 * src/common/model/element.h
 *
 * Copyright (c) 2021-2022 Bartek Kryza <bkryza@gmail.com>
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

#include "util/util.h"

#include <type_safe/optional_ref.hpp>

#include <string>
#include <vector>

namespace clanguml::common::model {
template <typename T> class nested_trait {
public:
    nested_trait() = default;

    nested_trait(const nested_trait &) = delete;
    nested_trait(nested_trait &&) = default;

    nested_trait &operator=(const nested_trait &) = delete;
    nested_trait &operator=(nested_trait &&) = default;

    virtual ~nested_trait() = default;

    void add_element(std::unique_ptr<T> p)
    {
        auto it = std::find_if(elements_.begin(), elements_.end(),
            [&p](const auto &e) { return *e == *p; });

        if (it != elements_.end()) {
            (*it)->append(*p);
        }
        else {
            elements_.emplace_back(std::move(p));
        }
    }

    void add_element(std::vector<std::string> path, std::unique_ptr<T> p)
    {
        assert(p);

        LOG_DBG("Adding nested element {} at path '{}'", p->name(),
            fmt::join(path, "::"));

        if (path.empty()) {
            add_element(std::move(p));
            return;
        }

        auto parent = get_element(path);

        if (parent)
            parent.value().add_element(std::move(p));
        else
            spdlog::error(
                "No parent element found at: {}", fmt::join(path, "::"));
    }

    type_safe::optional_ref<T> get_element(std::vector<std::string> path) const
    {
        LOG_DBG("Getting nested element at path: {}", fmt::join(path, "::"));

        if (path.empty() || !has_element(path.at(0))) {
            LOG_WARN("Nested element {} not found in element",
                fmt::join(path, "::"));
            return {};
        }

        auto p = get_element(path.at(0));
        if (path.size() == 1)
            return p;

        return p.value().get_element(
            std::vector<std::string>(path.begin() + 1, path.end()));
    }

    type_safe::optional_ref<T> get_element(const std::string &name) const
    {
        auto it = std::find_if(elements_.cbegin(), elements_.cend(),
            [&](const auto &p) { return name == p->name(); });

        if (it == elements_.end())
            return {};

        assert(it->get() != nullptr);

        return type_safe::ref(*(it->get()));
    }

    bool has_element(const std::string &name) const
    {
        return std::find_if(elements_.cbegin(), elements_.cend(),
                   [&](const auto &p) { return name == p->name(); }) !=
            elements_.end();
    }

    auto begin() { return elements_.begin(); }
    auto end() { return elements_.end(); }

    auto cbegin() const { return elements_.cbegin(); }
    auto cend() const { return elements_.cend(); }

    auto begin() const { return elements_.begin(); }
    auto end() const { return elements_.end(); }

private:
    std::vector<std::unique_ptr<T>> elements_;
};

}