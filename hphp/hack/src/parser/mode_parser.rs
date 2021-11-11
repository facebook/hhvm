// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bumpalo::Bump;
use oxidized::file_info::Mode;
use parser_core_types::{
    parser_env::ParserEnv,
    source_text::SourceText,
    syntax_by_ref::{
        syntax::Syntax,
        syntax_variant_generated::{self as syntax, SyntaxVariant},
    },
};

pub enum Language {
    Hack,
    PHP,
}

// Returns Language::PHP if text is a PHP file (has .php extension and starts
// with anything other than <?hh). Correctly recognizing PHP files is important
// for open-source projects, which may need Hack and PHP files to coexist in the
// same directory.
pub fn parse_mode(text: &SourceText<'_>) -> (Language, Option<Mode>) {
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
                            // <?, <?php or <?anything_else except <?hh
                            SyntaxVariant::Missing => (Language::PHP, None),
                            // <?hh
                            _ => {
                                if text.file_path().has_extension("hhi") {
                                    (Language::Hack, Some(Mode::Mhhi))
                                } else {
                                    (Language::Hack, Some(Mode::Mstrict))
                                }
                            }
                        }
                    }
                    _ => {
                        // unreachable (parser either returns a value that matches
                        // the above expressions, or None)
                        (Language::Hack, Some(Mode::Mstrict))
                    }
                }
            }
            // unreachable (parser either returns a value that matches
            // the above expression, or None)
            _ => (Language::Hack, Some(Mode::Mstrict)),
        }
    } else {
        (Language::Hack, Some(Mode::Mstrict))
    }
}
