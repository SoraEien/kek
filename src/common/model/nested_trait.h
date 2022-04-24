/**
 * src/common/model/nested_trait.h
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

#include <iostream>
#include <string>
#include <vector>

namespace clanguml::common::model {
template <typename T, typename Path> class nested_trait {
public:
    nested_trait() = default;

    nested_trait(const nested_trait &) = delete;
    nested_trait(nested_trait &&) = default;

    nested_trait &operator=(const nested_trait &) = delete;
    nested_trait &operator=(nested_trait &&) = default;

    virtual ~nested_trait() = default;

    template <typename V = T> void add_element(std::unique_ptr<V> p)
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

    template <typename V = T>
    void add_element(const Path &path, std::unique_ptr<V> p)
    {
        assert(p);

        LOG_DBG(
            "Adding nested element {} at path {}", p->name(), path.to_string());

        if (path.is_empty()) {
            add_element(std::move(p));
            return;
        }

        auto parent = get_element(path);

        if (parent && dynamic_cast<nested_trait<T, Path> *>(&parent.value()))
            dynamic_cast<nested_trait<T, Path> &>(parent.value())
                .template add_element<V>(std::move(p));
        else {
            spdlog::error("No parent element found at: {}", path.to_string());
            throw std::runtime_error(
                "No parent element found for " + path.to_string());
        }
    }

    template <typename V = T> auto get_element(const Path &path) const
    {
        LOG_DBG("Getting nested element at path: {}", path.to_string());

        if (path.is_empty() || !has_element(path[0])) {
            LOG_DBG("Nested element {} not found in element", path.to_string());
            return type_safe::optional_ref<V>{};
        }

        if (path.size() == 1) {
            return get_element<V>(path[0]);
        }

        auto p = get_element<T>(path[0]);

        if (!p)
            return type_safe::optional_ref<V>{};

        if (dynamic_cast<nested_trait<T, Path> *>(&p.value()))
            return dynamic_cast<nested_trait<T, Path> &>(p.value())
                .get_element<V>(Path{path.begin() + 1, path.end()});

        return type_safe::optional_ref<V>{};
    }

    template <typename V = T> auto get_element_parent(const T &element) const
    {
        auto path = element.path();
        auto parent = get_element(path);

        if (parent.has_value())
            return type_safe::optional_ref<V>{
                type_safe::ref<V>(dynamic_cast<V &>(parent.value()))};

        return type_safe::optional_ref<V>{};
    }

    template <typename V = T> auto get_element(const std::string &name) const
    {
        assert(!util::contains(name, "::"));

        auto it = std::find_if(elements_.cbegin(), elements_.cend(),
            [&](const auto &p) { return name == p->name(); });

        if (it == elements_.end())
            return type_safe::optional_ref<V>{type_safe::nullopt};

        assert(it->get() != nullptr);

        if (dynamic_cast<V *>(it->get()))
            return type_safe::optional_ref<V>{
                type_safe::ref<V>(dynamic_cast<V &>(*it->get()))};

        return type_safe::optional_ref<V>{type_safe::nullopt};
    }

    bool has_element(const std::string &name) const
    {
        return std::find_if(elements_.cbegin(), elements_.cend(),
                   [&](const auto &p) { return name == p->name(); }) !=
            elements_.end();
    }

    bool is_empty() const { return elements_.empty(); }

    auto begin() { return elements_.begin(); }
    auto end() { return elements_.end(); }

    auto cbegin() const { return elements_.cbegin(); }
    auto cend() const { return elements_.cend(); }

    auto begin() const { return elements_.begin(); }
    auto end() const { return elements_.end(); }

    void print_tree(const int level)
    {
        const auto &d = *this;

        if (level == 0) {
            std::cout << "--- Printing tree:\n";
        }
        for (const auto &e : d) {
            if (dynamic_cast<nested_trait<T, Path> *>(e.get())) {
                std::cout << std::string(level, ' ') << "[" << *e << "]\n";
                dynamic_cast<nested_trait<T, Path> *>(e.get())->print_tree(
                    level + 1);
            }
            else {
                std::cout << std::string(level, ' ') << "- " << *e << "]\n";
            }
        }
    }

private:
    std::vector<std::unique_ptr<T>> elements_;
};

}
