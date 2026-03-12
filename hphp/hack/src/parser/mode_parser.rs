// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bumpalo::Bump;
use parser_core_types::lexable_token::LexableToken;
use parser_core_types::parser_env::ParserEnv;
use parser_core_types::source_text::SourceText;
use parser_core_types::syntax_by_ref::syntax::Syntax;
use parser_core_types::syntax_by_ref::syntax_variant_generated::SyntaxVariant;
use parser_core_types::syntax_by_ref::syntax_variant_generated::{self as syntax};
use parser_core_types::syntax_tree::FileMode;

pub enum Language {
    Hack,
    PHP,
}

// Returns Language::PHP if text is a PHP file (has .php extension and starts
// with anything other than <?hh). Correctly recognizing PHP files is important
// for open-source projects, which may need Hack and PHP files to coexist in the
// same directory.
pub fn parse_mode(text: &SourceText<'_>) -> (Language, Option<FileMode>) {
    let bump = Bump::new();
    if let Some(header) =
        positioned_by_ref_parser::parse_header_only(ParserEnv::default(), &bump, text)
    {
        match header.children {
            SyntaxVariant::MarkupSection(section_children) => {
                match section_children {
                    syntax::MarkupSectionChildren {
                        suffix:
                            Syntax {
                                children: SyntaxVariant::MarkupSuffix(suffix_children),
                                ..
                            },
                        ..
                    } => {
                        let syntax::MarkupSuffixChildren { name, .. } = *suffix_children;
                        match &name.children {
                            SyntaxVariant::Token(token) => {
                                let name_text =
                                    text.sub_as_str(token.start_offset(), token.width());
                                if name_text.eq_ignore_ascii_case("hh") {
                                    if text.file_path().has_extension("hhi") {
                                        (Language::Hack, Some(FileMode::Hhi))
                                    } else {
                                        (Language::Hack, Some(FileMode::Strict))
                                    }
                                } else if name_text.eq_ignore_ascii_case("php") {
                                    (Language::PHP, None)
                                } else {
                                    // Invalid header like <?h or <?xyz — treat
                                    // as Hack so we report errors instead of
                                    // treating as PHP and silently ignoring the
                                    // file.
                                    (Language::Hack, Some(FileMode::Strict))
                                }
                            }
                            // Bare <? (PHP short tag) — treat as PHP
                            _ => (Language::PHP, None),
                        }
                    }
                    _ => {
                        // unreachable (parser either returns a value that matches
                        // the above expressions, or None)
                        (Language::Hack, Some(FileMode::Strict))
                    }
                }
            }
            // unreachable (parser either returns a value that matches
            // the above expression, or None)
            _ => (Language::Hack, Some(FileMode::Strict)),
        }
    } else {
        (Language::Hack, Some(FileMode::Strict))
    }
}
