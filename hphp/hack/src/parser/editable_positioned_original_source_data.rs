// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

/// SourceData represents information with relation to the original SourceText.
use parser_rust as parser;

use parser::lexable_token::LexableToken;
use parser::positioned_syntax::PositionedSyntax;
use parser::positioned_token::PositionedToken;
use parser::positioned_trivia::PositionedTrivia;
use parser::source_text::SourceText;
use parser_core_types::syntax_trait::SyntaxTrait;

// Data about the token with respect to the original source text.
#[derive(Clone)]
pub struct SourceData<'a> {
    pub source_text: SourceText<'a>,
    pub offset: usize, // Beginning of first trivia
    pub leading_width: usize,
    pub width: usize, // Width of actual token, not counting trivia
    pub trailing_width: usize,
    pub leading: Vec<PositionedTrivia>,
    pub trailing: Vec<PositionedTrivia>,
}

impl<'a> SourceData<'a> {
    pub fn from_positioned_token(
        positioned_token: &PositionedToken,
        source_text: &SourceText<'a>,
    ) -> Self {
        Self {
            source_text: source_text.clone(),
            offset: positioned_token.leading_start_offset().unwrap(),
            leading_width: positioned_token.leading_width(),
            width: positioned_token.width(),
            trailing_width: positioned_token.trailing_width(),
            leading: positioned_token.leading().to_vec(),
            trailing: positioned_token.trailing().to_vec(),
        }
    }

    pub fn from_positioned_syntax(syntax: &PositionedSyntax, source_text: &SourceText<'a>) -> Self {
        Self {
            source_text: source_text.clone(),
            offset: syntax.leading_start_offset(),
            leading_width: syntax.leading_width(),
            width: syntax.width(),
            trailing_width: syntax.trailing_width(),
            leading: [].to_vec(),
            trailing: [].to_vec(),
        }
    }
}
