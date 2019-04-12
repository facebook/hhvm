/**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*/
use crate::lexable_token::LexableToken;
use crate::parser::Parser;
use crate::parser_env::ParserEnv;
use crate::source_text::SourceText;
use crate::syntax::{self, SyntaxVariant};
use crate::token_kind::TokenKind;

pub enum FileMode {
    Mphp,          /* Do the best you can to support legacy PHP */
    Mdecl,         /* just declare signatures, don't check anything */
    Mstrict,       /* check everything! */
    Mpartial,      /* Don't fail if you see a function/class you don't know */
    Mexperimental, /* Strict mode + experimental features */
}

impl FileMode {
    fn from_string(s: &str) -> Option<Self> {
        match s {
            "strict" | "" => Some(FileMode::Mstrict),
            "partial" => Some(FileMode::Mpartial),
            "experimental" => Some(FileMode::Mexperimental),
            _ => None,
        }
    }
}

use crate::minimal_syntax::MinimalValue;
use crate::minimal_token::MinimalToken;
use crate::syntax_smart_constructors::SyntaxSmartConstructors;

type MinimalParser<'a> = Parser<'a, SyntaxSmartConstructors<MinimalToken, MinimalValue>>;

pub fn parse_mode(text: &SourceText) -> Option<FileMode> {
    let is_hhi = false; // TODO(kasper) : hhi files
    let header = MinimalParser::parse_header_only(ParserEnv::default(), text);
    match header {
        None => {
            Some(FileMode::Mstrict) /* no header - assume .hack file */
        }
        Some(header) => match header.syntax {
            SyntaxVariant::MarkupSection(box syntax::MarkupSectionChildren {
                markup_prefix: pfx,
                markup_text: txt,
                markup_suffix:
                    syntax::Syntax {
                        syntax:
                            SyntaxVariant::MarkupSuffix(box syntax::MarkupSuffixChildren {
                                markup_suffix_less_than_question: ltq,
                                markup_suffix_name: name,
                            }),
                        ..
                    },
                ..
            }) => match &name.syntax {
                SyntaxVariant::Missing => Some(FileMode::Mphp),
                SyntaxVariant::Token(t) if t.kind() == TokenKind::Equal => Some(FileMode::Mphp),
                _ => {
                    let skip_length = pfx.value.full_width
                        + txt.value.full_width
                        + ltq.value.full_width
                        + name.leading_width();

                    let language = text.sub_as_str(skip_length, name.width());
                    if language == "php" {
                        Some(FileMode::Mphp)
                    } else if is_hhi {
                        Some(FileMode::Mdecl)
                    } else {
                        let skip_length = skip_length + name.width();
                        let s = text.sub_as_str(skip_length, name.trailing_width()).trim();

                        let mut chars = s.chars();
                        let c0 = chars.next();
                        let c1 = chars.next();

                        let mode = if c0 != Some('/') || c1 != Some('/') {
                            return FileMode::from_string("");
                        } else {
                            chars.as_str()
                        };

                        let mode = match mode.trim().split_whitespace().next() {
                            None => "",
                            Some(mode) => mode,
                        };
                        FileMode::from_string(mode)
                    }
                }
            },
            _ => Some(FileMode::Mstrict),
        },
    }
}
