// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

/// SourceData represents information with relation to the original SourceText.
use parser_core_types::lexable_token::LexableToken;
use parser_core_types::positioned_syntax::PositionedSyntax;
use parser_core_types::positioned_token::PositionedToken;
use parser_core_types::positioned_trivia::PositionedTrivia;
use parser_core_types::source_text::SourceText;
use parser_core_types::syntax_trait::SyntaxTrait;

// Data about the token with respect to the original source text.
#[derive(Clone, Debug)]
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

    pub fn text(data: &Self) -> String {
        std::str::from_utf8(SourceText::sub(
            &data.source_text,
            data.offset + data.leading_width,
            data.width,
        ))
        .unwrap()
        .to_string()
    }

    pub fn spanning_between(b: &Self, e: &Self) -> Self {
        Self {
            source_text: b.source_text.clone(),
            offset: b.offset,
            leading_width: b.leading_width,
            width: Self::end_offset(e) + 1 - Self::start_offset(b),
            trailing_width: e.trailing_width,
            leading: b.leading.to_vec(),
            trailing: e.trailing.to_vec(),
        }
    }

    fn start_offset(data: &Self) -> usize {
        data.offset + data.leading_width
    }

    fn end_offset(data: &Self) -> usize {
        if data.width < 1 {
            Self::start_offset(data)
        } else {
            Self::start_offset(data) + data.width - 1
        }
    }
}
