// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use parser::{
    lexable_token::LexableToken, positioned_trivia::PositionedTrivium, source_text::SourceText,
    syntax_by_ref::positioned_token::PositionedToken, token_kind::TokenKind,
};

pub trait RescanTrivia<Trivium> {
    fn scan_leading(&self, source_text: &SourceText<'_>) -> Vec<Trivium>;
    fn scan_trailing(&self, source_text: &SourceText<'_>) -> Vec<Trivium>;
}

impl RescanTrivia<PositionedTrivium> for PositionedToken<'_> {
    fn scan_leading(&self, source_text: &SourceText<'_>) -> Vec<PositionedTrivium> {
        let f = if self.kind() == TokenKind::XHPBody {
            positioned_parser::scan_leading_xhp_trivia
        } else {
            positioned_parser::scan_leading_php_trivia
        };
        f(
            source_text,
            self.leading_start_offset().unwrap(),
            self.leading_width(),
        )
    }

    fn scan_trailing(&self, source_text: &SourceText<'_>) -> Vec<PositionedTrivium> {
        let f = if self.kind() == TokenKind::XHPBody {
            positioned_parser::scan_trailing_xhp_trivia
        } else {
            positioned_parser::scan_trailing_php_trivia
        };
        f(source_text, self.leading_start_offset().unwrap())
    }
}
