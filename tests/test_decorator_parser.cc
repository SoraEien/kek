/**
 * tests/test_decorator_parser.cc
 *
 * Copyright (c) 2021 Bartek Kryza <bkryza@gmail.com>
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
#define CATCH_CONFIG_MAIN

#include "uml/decorators.h"

#include "catch.h"

TEST_CASE("Test decorator parser on regular comment", "[unit-test]")
{
    std::string comment = R"(
    \brief This is a comment.

    This is a longer comment.

    \param a int an int
    \param b float a float

    )";

    using namespace clanguml::decorators;

    auto decorators = parse(comment);

    CHECK(decorators.empty());
}

TEST_CASE("Test decorator parser on note", "[unit-test]")
{
    std::string comment = R"(
    \brief This is a comment.

    This is a longer comment.

    \

    \param a int an int
    \param b float a float

    \clanguml{note[left] This is a comment}
    @clanguml{note[ top  ]

            This is a comment    }
    )";

    using namespace clanguml::decorators;

    auto decorators = parse(comment);

    CHECK(decorators.size() == 2);

    auto n1 = std::dynamic_pointer_cast<note>(decorators.at(0));
    auto n2 = std::dynamic_pointer_cast<note>(decorators.at(1));

    CHECK(n1);
    CHECK(n1->position == "left");
    CHECK(n1->text == "This is a comment");

    CHECK(n2);
    CHECK(n2->position == "top");
    CHECK(n2->text == "This is a comment");
}

TEST_CASE("Test decorator parser on style", "[unit-test]")
{
    std::string comment = R"(
    \clanguml{style[#green,dashed,thickness=4]}
    )";

    using namespace clanguml::decorators;

    auto decorators = parse(comment);

    CHECK(decorators.size() == 1);

    auto n1 = std::dynamic_pointer_cast<style>(decorators.at(0));

    CHECK(n1);
    CHECK(n1->spec == "#green,dashed,thickness=4");
}

TEST_CASE("Test decorator parser on aggregation", "[unit-test]")
{
    std::string comment = R"(
    \clanguml{aggregation[0..1:0..*]}
    )";

    using namespace clanguml::decorators;

    auto decorators = parse(comment);

    CHECK(decorators.size() == 1);

    auto n1 = std::dynamic_pointer_cast<aggregation>(decorators.at(0));

    CHECK(n1);
    CHECK(n1->multiplicity == "0..1:0..*");
}

TEST_CASE("Test decorator parser on skip", "[unit-test]")
{
    std::string comment = R"(
    \clanguml{skip}
    )";

    using namespace clanguml::decorators;

    auto decorators = parse(comment);

    CHECK(decorators.size() == 1);

    auto n1 = std::dynamic_pointer_cast<skip>(decorators.at(0));

    CHECK(n1);
}

TEST_CASE("Test decorator parser on skiprelationship", "[unit-test]")
{
    std::string comment = R"(
    \clanguml{skiprelationship}
    )";

    using namespace clanguml::decorators;

    auto decorators = parse(comment);

    CHECK(decorators.size() == 1);

    auto n1 = std::dynamic_pointer_cast<skip_relationship>(decorators.at(0));

    CHECK(n1);
}