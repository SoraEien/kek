/**
 * src/common/visitor/translation_unit_visitor.cc
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

#include "translation_unit_visitor.h"

#include "comment/clang_visitor.h"
#include "comment/plain_visitor.h"

namespace clanguml::common::visitor {

translation_unit_visitor::translation_unit_visitor(
    clang::SourceManager &sm, const clanguml::config::diagram &config)
    : source_manager_{sm}
    , relative_to_path_{config.relative_to()}
{
    if (config.comment_parser() == config::comment_parser_t::plain) {
        comment_visitor_ =
            std::make_unique<comment::plain_visitor>(source_manager_);
    }
    else if (config.comment_parser() == config::comment_parser_t::clang) {
        comment_visitor_ =
            std::make_unique<comment::clang_visitor>(source_manager_);
    }
}

clang::SourceManager &translation_unit_visitor::source_manager() const
{
    return source_manager_;
}

void translation_unit_visitor::process_comment(
    const clang::NamedDecl &decl, clanguml::common::model::decorated_element &e)
{

    assert(comment_visitor_.get() != nullptr);

    comment_visitor_->visit(decl, e);

    const auto *comment =
        decl.getASTContext().getRawCommentForDeclNoCache(&decl);

    if (comment != nullptr) {
        // Process clang-uml decorators in the comments
        // TODO: Refactor to use standard block comments processable by clang
        //       comments
        e.add_decorators(decorators::parse(comment->getFormattedText(
            source_manager_, decl.getASTContext().getDiagnostics())));
    }
}

void translation_unit_visitor::set_source_location(
    const clang::Decl &decl, clanguml::common::model::source_location &element)
{
    set_source_location(decl.getLocation(), element);
}

void translation_unit_visitor::set_source_location(
    const clang::Expr &expr, clanguml::common::model::source_location &element)
{
    set_source_location(expr.getBeginLoc(), element);
}

void translation_unit_visitor::set_source_location(
    const clang::Stmt &stmt, clanguml::common::model::source_location &element)
{
    set_source_location(stmt.getBeginLoc(), element);
}

void translation_unit_visitor::set_source_location(
    const clang::SourceLocation &location,
    clanguml::common::model::source_location &element)
{
    std::string file;
    unsigned line{};
    [[maybe_unused]] unsigned column{};

    if (location.isValid()) {
        file = source_manager_.getFilename(location).str();
        line = source_manager_.getSpellingLineNumber(location);
        column = source_manager_.getSpellingColumnNumber(location);

        if (file.empty()) {
            // Why do I have to do this?
            parse_source_location(
                location.printToString(source_manager()), file, line, column);
        }
    }
    else {
        auto success = parse_source_location(
            location.printToString(source_manager()), file, line, column);
        if (!success) {
            LOG_DBG("Failed to extract source location for element from {}",
                location.printToString(source_manager_));
            return;
        }
    }

    element.set_file(file);
    element.set_file_relative(util::path_to_url(
        std::filesystem::relative(element.file(), relative_to_path_).string()));
    element.set_line(line);
    element.set_location_id(location.getHashValue());
}

} // namespace clanguml::common::visitor