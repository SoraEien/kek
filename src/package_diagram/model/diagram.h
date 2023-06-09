/**
 * src/package_diagram/model/diagram.h
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

#include "common/model/diagram.h"
#include "common/model/element_view.h"
#include "common/model/package.h"

#include <string>
#include <vector>

namespace clanguml::package_diagram::model {

using clanguml::common::opt_ref;
using clanguml::common::model::diagram_element;
using clanguml::common::model::package;
using clanguml::common::model::path;

class diagram : public clanguml::common::model::diagram,
                public clanguml::common::model::element_view<package>,
                public clanguml::common::model::nested_trait<
                    clanguml::common::model::element,
                    clanguml::common::model::namespace_> {
public:
    diagram() = default;

    diagram(const diagram &) = delete;
    diagram(diagram &&) = default;
    diagram &operator=(const diagram &) = delete;
    diagram &operator=(diagram &&) = default;

    common::model::diagram_t type() const override;

    const common::reference_vector<package> &packages() const;

    opt_ref<diagram_element> get(const std::string &full_name) const override;

    opt_ref<diagram_element> get(diagram_element::id_t id) const override;

    template <typename ElementT>
    opt_ref<ElementT> find(const std::string &name) const;

    template <typename ElementT>
    opt_ref<ElementT> find(diagram_element::id_t id) const;

    template <typename ElementT>
    bool add(const path &parent_path, std::unique_ptr<ElementT> &&e)
    {
        if (parent_path.type() == common::model::path_type::kNamespace) {
            return add_with_namespace_path(std::move(e));
        }

        return add_with_filesystem_path(parent_path, std::move(e));
    }

    template <typename ElementT>
    bool add_with_namespace_path(std::unique_ptr<ElementT> &&e);

    template <typename ElementT>
    bool add_with_filesystem_path(
        const common::model::path &parent_path, std::unique_ptr<ElementT> &&e);

    std::string to_alias(diagram_element::id_t /*id*/) const;

    inja::json context() const override;
};

template <typename ElementT>
opt_ref<ElementT> diagram::find(const std::string &name) const
{
    for (const auto &element : element_view<ElementT>::view()) {
        const auto full_name = element.get().full_name(false);

        if (full_name == name) {
            return {element};
        }
    }

    return {};
}

template <typename ElementT>
opt_ref<ElementT> diagram::find(diagram_element::id_t id) const
{
    for (const auto &element : element_view<ElementT>::view()) {
        if (element.get().id() == id) {
            return {element};
        }
    }

    return {};
}

template <typename ElementT>
bool diagram::add_with_namespace_path(std::unique_ptr<ElementT> &&p)
{
    LOG_DBG(
        "Adding package: {}, {}, [{}]", p->name(), p->full_name(true), p->id());

    auto ns = p->get_relative_namespace();
    auto p_ref = std::ref(*p);

    auto res = add_element(ns, std::move(p));
    if (res)
        element_view<ElementT>::add(p_ref);

    return res;
}

template <typename ElementT>
bool diagram::add_with_filesystem_path(
    const common::model::path &parent_path, std::unique_ptr<ElementT> &&p)
{
    LOG_DBG("Adding package: {}, {}", p->name(), p->full_name(true));

    // Make sure all parent directories are already packages in the
    // model
    for (auto it = parent_path.begin(); it != parent_path.end(); it++) {
        auto pkg =
            std::make_unique<common::model::package>(p->using_namespace());
        pkg->set_name(*it);
        auto ns = common::model::path(parent_path.begin(), it);
        pkg->set_namespace(ns);
        pkg->set_id(common::to_id(pkg->full_name(false)));

        add_with_filesystem_path(ns, std::move(pkg));
    }

    auto pp = std::ref(*p);
    auto res = add_element(parent_path, std::move(p));
    if (res)
        element_view<ElementT>::add(pp);

    return res;
}

} // namespace clanguml::package_diagram::model

namespace clanguml::common::model {
template <>
bool check_diagram_type<clanguml::package_diagram::model::diagram>(diagram_t t);
}
