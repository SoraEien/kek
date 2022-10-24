/**
 * src/sequence_diagram/model/participant.cc
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

#include "participant.h"

namespace clanguml::sequence_diagram::model {
class_::class_(const common::model::namespace_ &using_namespace)
    : participant{using_namespace}
{
}

bool class_::is_struct() const { return is_struct_; }

void class_::is_struct(bool is_struct) { is_struct_ = is_struct; }

bool class_::is_template() const { return is_template_; }

void class_::is_template(bool is_template) { is_template_ = is_template; }

bool class_::is_template_instantiation() const
{
    return is_template_instantiation_;
}

void class_::is_template_instantiation(bool is_template_instantiation)
{
    is_template_instantiation_ = is_template_instantiation;
}

void class_::add_template(class_diagram::model::template_parameter tmplt)
{
    templates_.emplace_back(std::move(tmplt));
}

const std::vector<class_diagram::model::template_parameter> &
class_::templates() const
{
    return templates_;
}

std::string class_::full_name_no_ns() const
{
    using namespace clanguml::util;

    std::ostringstream ostr;

    ostr << name();

    render_template_params(ostr, false);

    return ostr.str();
}

std::string class_::full_name(bool relative) const
{
    using namespace clanguml::util;
    using clanguml::common::model::namespace_;

    std::ostringstream ostr;

    ostr << name_and_ns();
    render_template_params(ostr, relative);

    std::string res;

    if (relative)
        res = using_namespace().relative(ostr.str());
    else
        res = ostr.str();

    if (res.empty())
        return "<<anonymous>>";

    return res;
}

std::ostringstream &class_::render_template_params(
    std::ostringstream &ostr, bool relative) const
{
    using clanguml::common::model::namespace_;

    if (!templates_.empty()) {
        std::vector<std::string> tnames;
        std::vector<std::string> tnames_simplified;

        std::transform(templates_.cbegin(), templates_.cend(),
            std::back_inserter(tnames),
            [ns = using_namespace(), relative](
                const auto &tmplt) { return tmplt.to_string(ns, relative); });

        ostr << fmt::format("<{}>", fmt::join(tnames, ","));
    }

    return ostr;
}

function::function(const common::model::namespace_ &using_namespace)
    : participant{using_namespace}
{
}

std::string function::full_name(bool relative) const
{
    return element::full_name(relative) + "()";
}

std::string function::full_name_no_ns() const
{
    return element::full_name_no_ns() + "()";
}

method::method(const common::model::namespace_ &using_namespace)
    : participant{using_namespace}
{
}

std::string method::alias() const
{
    assert(class_id_ >= 0);

    return fmt::format("C_{:022}", class_id_);
}
}