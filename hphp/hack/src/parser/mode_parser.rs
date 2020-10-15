// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized::file_info::Mode;
use parser_core_types::{
    parser_env::ParserEnv,
    source_text::SourceText,
    syntax::{self, SyntaxVariant},
};

pub enum Language {
    Hack,
    PHP,
}

// Returns Language::PHP if text is a PHP file (has .php extension and starts
// with anything other than <?hh). Correctly recognizing PHP files is important
// for open-source projects, which may need Hack and PHP files to coexist in the
// same directory.
pub fn parse_mode(text: &SourceText) -> (Language, Option<Mode>) {
    if let Some(header) = minimal_parser::parse_header_only(ParserEnv::default(), text) {
        match header.syntax {
            SyntaxVariant::MarkupSection(section_children) => {
                if let syntax::MarkupSectionChildren {
                    markup_hashbang: hashbang,
                    markup_suffix:
                        syntax::Syntax {
                            syntax: SyntaxVariant::MarkupSuffix(suffix_children),
                            ..
                        },
                    ..
                } = *section_children
                {
                    let syntax::MarkupSuffixChildren {
                        markup_suffix_less_than_question: ltq,
                        markup_suffix_name: name,
                    } = *suffix_children;
                    return match &name.syntax {
                        // <?, <?php or <?anything_else except <?hh
                        SyntaxVariant::Missing => (Language::PHP, None),
                        // <?hh optionally followed by // mode
                        _ => {
                            if text.file_path().has_extension("hhi") {
                                (Language::Hack, Some(Mode::Mdecl))
                            } else {
                                let skip_length = hashbang.value.full_width
                                    + ltq.value.full_width
                                    + name.leading_width()
                                    + name.width();
                                let s = text.sub_as_str(skip_length, name.trailing_width()).trim();

                                let mut chars = s.chars();
                                let c0 = chars.next();
                                let c1 = chars.next();

                                let mode = if c0 != Some('/') || c1 != Some('/') {
                                    return (Language::Hack, Mode::from_string(""));
                                } else {
                                    chars.as_str()
                                };

                                let mode = match mode.trim().split_whitespace().next() {
                                    None => "",
                                    Some(mode) => mode,
                                };
                                (Language::Hack, Mode::from_string(mode))
                            }
                        }
                    };
                } else {
                    // unreachable (parser either returns a value that matches
                    // the above expression, or None)
                    (Language::Hack, Some(Mode::Mstrict))
                }
            }
            // unreachable (parser either returns a value that matches
            // the above expression, or None)
            _ => (Language::Hack, Some(Mode::Mstrict)),
        }
    } else if text.file_path().has_extension("php")
    /* TODO: add hhconfig option */
    {
        // File doesn't start with <?. This is valid PHP, since PHP files may
        // contain <?, <?php, <?= in the middle or not at all.
        // TODO: Should return None but there are currently various tests (and
        // possibly non-tests) that depend on this broken behavior:
        (Language::Hack, Some(Mode::Mstrict))
    } else if text.file_path().has_extension("hackpartial") {
        (Language::Hack, Some(Mode::Mpartial))
    } else {
        (Language::Hack, Some(Mode::Mstrict))
    }
}
