// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use parser_core_types::lexable_token::LexableToken;
use parser_core_types::positioned_token::PositionedToken;
use parser_core_types::positioned_trivia::PositionedTrivia;
use parser_core_types::source_text::SourceText;
use parser_core_types::token_kind::TokenKind;
use parser_core_types::trivia_kind::TriviaKind;

use crate::editable_positioned_original_source_data::SourceData;

#[derive(Clone, Debug)]
pub struct SyntheticTokenData {
    pub text: String,
}

#[derive(Clone, Debug)]
pub enum TokenData<'a> {
    Original(SourceData<'a>),
    SynthesizedFromOriginal(SyntheticTokenData, SourceData<'a>),
    Synthetic(SyntheticTokenData),
}

#[derive(Clone, Debug)]
pub struct EditablePositionedToken<'a> {
    pub kind: TokenKind,
    pub leading_text: String,
    pub trailing_text: String,
    pub token_data: TokenData<'a>,
}

impl<'a> LexableToken<'a> for EditablePositionedToken<'a> {
    type Trivia = PositionedTrivia;

    fn make(
        _kind: TokenKind,
        _source_text: &SourceText,
        _offset: usize,
        _width: usize,
        _leading: Vec<Self::Trivia>,
        _trailing: Vec<Self::Trivia>,
    ) -> Self {
        panic!("TODO")
    }

    fn kind(&self) -> TokenKind {
        self.kind
    }

    fn leading_start_offset(&self) -> Option<usize> {
        panic!("TODO")
    }

    fn width(&self) -> usize {
        panic!("TODO")
    }

    fn leading_width(&self) -> usize {
        panic!("TODO")
    }

    fn trailing_width(&self) -> usize {
        panic!("TODO")
    }

    fn full_width(&self) -> usize {
        panic!("TODO")
    }

    fn leading(&self) -> &[Self::Trivia] {
        panic!("TODO")
    }

    fn trailing(&self) -> &[Self::Trivia] {
        panic!("TODO")
    }

    fn with_leading(self, _leading: Vec<Self::Trivia>) -> Self {
        panic!("TODO")
    }

    fn with_trailing(self, _trailing: Vec<Self::Trivia>) -> Self {
        panic!("TODO")
    }

    fn with_kind(self, _kind: TokenKind) -> Self {
        panic!("TODO")
    }

    fn has_trivia_kind(&self, _kind: TriviaKind) -> bool {
        panic!("TODO")
    }
}

impl<'a> EditablePositionedToken<'a> {
    pub fn from_positioned_token(
        positioned_token: &PositionedToken,
        source_text: &SourceText<'a>,
    ) -> Self {
        let leading_text =
            String::from_utf8(positioned_token.leading_text(source_text).to_vec()).unwrap();
        let trailing_text =
            String::from_utf8(positioned_token.trailing_text(source_text).to_vec()).unwrap();
        Self {
            kind: positioned_token.kind(),
            leading_text,
            trailing_text,
            token_data: TokenData::Original(SourceData::from_positioned_token(
                positioned_token,
                source_text,
            )),
        }
    }

    pub fn synthesize_new(
        kind: TokenKind,
        leading_text: String,
        trailing_text: String,
        text: String,
    ) -> Self {
        let token_data = TokenData::Synthetic(SyntheticTokenData { text });
        Self {
            kind,
            leading_text,
            trailing_text,
            token_data,
        }
    }

    pub fn full_text(token: &Self) -> String {
        [
            token.leading_text.to_string(),
            Self::text(token),
            token.trailing_text.to_string(),
        ]
        .join("")
    }

    pub fn text(token: &Self) -> String {
        match &token.token_data {
            TokenData::Original(source_data) => SourceData::text(source_data),
            TokenData::SynthesizedFromOriginal(token_data, _)
            | TokenData::Synthetic(token_data) => token_data.text.clone(),
        }
    }
}
