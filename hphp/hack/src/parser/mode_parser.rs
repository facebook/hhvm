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

pub fn parse_mode(text: &SourceText) -> Option<Mode> {
    if let Some(header) = minimal_parser::parse_header_only(ParserEnv::default(), text) {
        match header.syntax {
            SyntaxVariant::MarkupSection(section_children) => {
                if let syntax::MarkupSectionChildren {
                    markup_text: txt,
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
                        SyntaxVariant::Missing => None,
                        _ => {
                            if text.file_path().has_extension("hhi") {
                                Some(Mode::Mdecl)
                            } else {
                                let skip_length = txt.value.full_width
                                    + ltq.value.full_width
                                    + name.leading_width()
                                    + name.width();
                                let s = text.sub_as_str(skip_length, name.trailing_width()).trim();

                                let mut chars = s.chars();
                                let c0 = chars.next();
                                let c1 = chars.next();

                                let mode = if c0 != Some('/') || c1 != Some('/') {
                                    return Mode::from_string("");
                                } else {
                                    chars.as_str()
                                };

                                let mode = match mode.trim().split_whitespace().next() {
                                    None => "",
                                    Some(mode) => mode,
                                };
                                Mode::from_string(mode)
                            }
                        }
                    };
                } else {
                    Some(Mode::Mstrict)
                }
            }
            _ => Some(Mode::Mstrict),
        }
    } else {
        if text.file_path().has_extension("hackpartial") {
            Some(Mode::Mpartial)
        } else {
            Some(Mode::Mstrict)
        }
    }
}
