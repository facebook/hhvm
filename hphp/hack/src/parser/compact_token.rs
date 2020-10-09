// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::convert::TryInto;

use crate::{
    compact_trivia::{CompactTrivia, TriviaKinds},
    lexable_token::LexableToken,
    source_text::SourceText,
    token_factory::SimpleTokenFactory,
    token_kind::TokenKind,
    trivia_kind::TriviaKind,
};

/// A compact version of parser_core_types::PositionedToken. Most tokens will be
/// represented inline in 16 bytes (the same size as a slice). Tokens which do
/// not fit in that representation will allocate a Box for their contents.
#[derive(Debug, Clone, PartialEq)]
pub struct CompactToken {
    repr: CompactTokenRepr,
}

#[derive(Debug, Clone, PartialEq)]
enum CompactTokenRepr {
    Small(SmallToken),
    Large(Box<LargeToken>),
}
use CompactTokenRepr::*;

#[derive(Debug, Clone, PartialEq)]
struct SmallToken {
    kind: TokenKind,
    offset: u32, // Points to the first byte of the first leading trivium
    leading_width: u16,
    width: u16, // Width of actual token, not counting trivia
    trailing_width: u8,
    leading: TriviaKinds,
    trailing: TriviaKinds,
}

#[derive(Debug, Clone, PartialEq)]
struct LargeToken {
    kind: TokenKind,
    offset: usize, // Points to the first byte of the first leading trivium
    leading_width: usize,
    width: usize, // Width of actual token, not counting trivia
    trailing_width: usize,
    leading: TriviaKinds,
    trailing: TriviaKinds,
}

impl CompactToken {
    pub fn new(
        kind: TokenKind,
        offset: usize,
        width: usize,
        leading: CompactTrivia,
        trailing: CompactTrivia,
    ) -> Self {
        let leading_width = leading.width;
        let leading = leading.kinds;
        let trailing_width = trailing.width;
        let trailing = trailing.kinds;

        match (
            offset.try_into(),
            leading_width.try_into(),
            width.try_into(),
            trailing_width.try_into(),
        ) {
            (Ok(o), Ok(lw), Ok(w), Ok(tw)) => Self {
                repr: Small(SmallToken {
                    kind,
                    offset: o,
                    leading_width: lw,
                    width: w,
                    trailing_width: tw,
                    leading,
                    trailing,
                }),
            },
            _ => Self {
                repr: Large(Box::new(LargeToken {
                    kind,
                    offset,
                    leading_width,
                    width,
                    trailing_width,
                    leading,
                    trailing,
                })),
            },
        }
    }

    pub fn kind(&self) -> TokenKind {
        match &self.repr {
            &Small(SmallToken { kind, .. }) => kind,
            Large(token) => token.kind,
        }
    }

    /// Private because of the ambiguous name. This is the offset field on
    /// SmallToken and LargeToken--the byte offset of the first leading trivium.
    fn offset(&self) -> usize {
        match &self.repr {
            &Small(SmallToken { offset, .. }) => offset as usize,
            Large(token) => token.offset,
        }
    }

    pub fn width(&self) -> usize {
        match &self.repr {
            &Small(SmallToken { width, .. }) => width as usize,
            Large(token) => token.width,
        }
    }

    pub fn leading_width(&self) -> usize {
        match &self.repr {
            &Small(SmallToken { leading_width, .. }) => leading_width as usize,
            Large(token) => token.leading_width,
        }
    }

    pub fn trailing_width(&self) -> usize {
        match &self.repr {
            &Small(SmallToken { trailing_width, .. }) => trailing_width as usize,
            Large(token) => token.trailing_width,
        }
    }

    pub fn leading_trivia(&self) -> CompactTrivia {
        match &self.repr {
            Small(token) => CompactTrivia::make(token.leading, token.leading_width as usize),
            Large(token) => CompactTrivia::make(token.leading, token.leading_width),
        }
    }

    pub fn trailing_trivia(&self) -> CompactTrivia {
        match &self.repr {
            Small(token) => CompactTrivia::make(token.trailing, token.trailing_width as usize),
            Large(token) => CompactTrivia::make(token.trailing, token.trailing_width),
        }
    }

    pub fn leading_is_empty(&self) -> bool {
        match &self.repr {
            Small(token) => token.leading.is_empty(),
            Large(token) => token.leading.is_empty(),
        }
    }

    pub fn trailing_is_empty(&self) -> bool {
        match &self.repr {
            Small(token) => token.trailing.is_empty(),
            Large(token) => token.trailing.is_empty(),
        }
    }

    pub fn start_offset(&self) -> usize {
        self.offset() + self.leading_width()
    }

    pub fn end_offset(&self) -> usize {
        self.start_offset() + self.width()
    }

    pub fn leading_start_offset(&self) -> usize {
        self.offset()
    }

    pub fn trailing_start_offset(&self) -> usize {
        self.end_offset()
    }

    pub fn leading_text<'a>(&self, source_text: &SourceText<'a>) -> &'a [u8] {
        source_text.sub(self.leading_start_offset(), self.leading_width())
    }

    pub fn trailing_text<'a>(&self, source_text: &SourceText<'a>) -> &'a [u8] {
        source_text.sub(self.trailing_start_offset(), self.trailing_width())
    }

    pub fn text<'a>(&self, source_text: &SourceText<'a>) -> &'a [u8] {
        source_text.sub(self.start_offset(), self.width())
    }

    pub fn has_leading_trivia_kind(&self, kind: TriviaKind) -> bool {
        match &self.repr {
            Small(token) => token.leading.has_kind(kind),
            Large(token) => token.leading.has_kind(kind),
        }
    }

    pub fn has_trailing_trivia_kind(&self, kind: TriviaKind) -> bool {
        match &self.repr {
            Small(token) => token.trailing.has_kind(kind),
            Large(token) => token.trailing.has_kind(kind),
        }
    }
}

impl LexableToken for CompactToken {
    type Trivia = CompactTrivia;

    fn kind(&self) -> TokenKind {
        self.kind()
    }

    fn leading_start_offset(&self) -> Option<usize> {
        Some(self.leading_start_offset())
    }

    fn width(&self) -> usize {
        self.width()
    }

    fn leading_width(&self) -> usize {
        self.leading_width()
    }

    fn trailing_width(&self) -> usize {
        self.trailing_width()
    }

    fn full_width(&self) -> usize {
        self.leading_width() + self.width() + self.trailing_width()
    }

    fn clone_leading(&self) -> CompactTrivia {
        self.leading_trivia()
    }

    fn clone_trailing(&self) -> CompactTrivia {
        self.trailing_trivia()
    }

    fn leading_is_empty(&self) -> bool {
        self.leading_is_empty()
    }

    fn trailing_is_empty(&self) -> bool {
        self.trailing_is_empty()
    }

    fn has_leading_trivia_kind(&self, kind: TriviaKind) -> bool {
        self.has_leading_trivia_kind(kind)
    }

    fn has_trailing_trivia_kind(&self, kind: TriviaKind) -> bool {
        self.has_trailing_trivia_kind(kind)
    }

    fn into_trivia_and_width(self) -> (Self::Trivia, usize, Self::Trivia) {
        match self.repr {
            Small(t) => (
                CompactTrivia::make(t.leading, t.leading_width as usize),
                t.width as usize,
                CompactTrivia::make(t.trailing, t.trailing_width as usize),
            ),
            Large(t) => (
                CompactTrivia::make(t.leading, t.leading_width),
                t.width,
                CompactTrivia::make(t.trailing, t.trailing_width),
            ),
        }
    }
}

impl SimpleTokenFactory for CompactToken {
    fn make(
        kind: TokenKind,
        offset: usize,
        width: usize,
        leading: CompactTrivia,
        trailing: CompactTrivia,
    ) -> Self {
        Self::new(kind, offset, width, leading, trailing)
    }

    fn with_leading(mut self, leading: CompactTrivia) -> Self {
        match &mut self.repr {
            Large(token) => {
                token.leading_width = leading.width;
                token.leading = leading.kinds;
            }
            Small(token) => {
                if let Ok(leading_width) = leading.width.try_into() {
                    token.leading_width = leading_width;
                    token.leading = leading.kinds;
                } else {
                    return Self {
                        repr: Large(Box::new(LargeToken {
                            kind: token.kind,
                            offset: token.offset as usize,
                            leading_width: leading.width,
                            width: token.width as usize,
                            trailing_width: token.trailing_width as usize,
                            leading: leading.kinds,
                            trailing: token.trailing,
                        })),
                    };
                }
            }
        }
        self
    }

    fn with_trailing(mut self, trailing: CompactTrivia) -> Self {
        match &mut self.repr {
            Large(token) => {
                token.trailing_width = trailing.width;
                token.trailing = trailing.kinds;
            }
            Small(token) => {
                if let Ok(trailing_width) = trailing.width.try_into() {
                    token.trailing_width = trailing_width;
                    token.trailing = trailing.kinds;
                } else {
                    return Self {
                        repr: Large(Box::new(LargeToken {
                            kind: token.kind,
                            offset: token.offset as usize,
                            leading_width: token.leading_width as usize,
                            width: token.width as usize,
                            trailing_width: trailing.width,
                            leading: token.leading,
                            trailing: trailing.kinds,
                        })),
                    };
                }
            }
        }
        self
    }

    fn with_kind(mut self, kind: TokenKind) -> Self {
        match &mut self.repr {
            Small(t) => t.kind = kind,
            Large(t) => t.kind = kind,
        }
        self
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn test_size() {
        // CompactToken is 16 bytes, the same size as a slice (on a 64-bit
        // architecture). If we end up needing to change this size, we should
        // carefully consider the performance impact on the direct decl parser.
        assert_eq!(std::mem::size_of::<CompactToken>(), 16);
    }
}
