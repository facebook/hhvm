// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocaml::core::mlvalues::{empty_list, Value};

use std::iter::Iterator;

use ocamlpool_rust::utils::*;
use ocamlrep_ocamlpool::add_to_ambient_pool;
use parser_core_types::{
    lexable_token::LexableToken,
    minimal_trivia::MinimalTrivia,
    positioned_trivia::PositionedTrivium,
    source_text::SourceText,
    syntax::*,
    syntax_by_ref::{
        arena_state::State as ArenaState,
        positioned_token::PositionedToken as PositionedTokenByRef,
        positioned_value::PositionedValue as PositionedValueByRef, syntax::Syntax as SyntaxByRef,
        syntax_variant_generated::SyntaxVariant as SyntaxVariantByRef,
    },
    syntax_error::SyntaxError,
    syntax_kind::SyntaxKind,
    token_kind::TokenKind,
    trivia_kind::TriviaKind,
};
use smart_constructors::NoState;

pub struct SerializationContext {
    pub source_text: Value,
}

impl SerializationContext {
    pub fn new(source_text: Value) -> Self {
        Self { source_text }
    }
}

pub trait ToOcaml {
    unsafe fn to_ocaml(&self, context: &SerializationContext) -> Value;
}

pub unsafe fn to_list<T>(values: &[T], context: &SerializationContext) -> Value
where
    T: ToOcaml,
{
    let mut res = empty_list();

    for v in values.iter().rev() {
        let tmp = res;
        res = reserve_block(0.into(), 2);
        caml_set_field(res, 0, v.to_ocaml(context));
        caml_set_field(res, 1, tmp);
    }
    res
}

impl ToOcaml for bool {
    unsafe fn to_ocaml(&self, _context: &SerializationContext) -> Value {
        add_to_ambient_pool(self)
    }
}

impl ToOcaml for Vec<bool> {
    unsafe fn to_ocaml(&self, _context: &SerializationContext) -> Value {
        add_to_ambient_pool(self)
    }
}

impl ToOcaml for TokenKind {
    unsafe fn to_ocaml(&self, _context: &SerializationContext) -> Value {
        add_to_ambient_pool(self)
    }
}

impl ToOcaml for TriviaKind {
    unsafe fn to_ocaml(&self, _context: &SerializationContext) -> Value {
        add_to_ambient_pool(self)
    }
}

impl ToOcaml for MinimalTrivia {
    unsafe fn to_ocaml(&self, _context: &SerializationContext) -> Value {
        add_to_ambient_pool(self)
    }
}

impl<Token, SyntaxValue> ToOcaml for SyntaxByRef<'_, Token, SyntaxValue>
where
    Token: LexableToken + ToOcaml,
    SyntaxValue: SyntaxValueType<Token> + ToOcaml,
{
    unsafe fn to_ocaml(&self, context: &SerializationContext) -> Value {
        let value = self.value.to_ocaml(context);

        let syntax = match &self.children {
            SyntaxVariantByRef::Missing => u8_to_ocaml(SyntaxKind::Missing.ocaml_tag()),
            SyntaxVariantByRef::Token(token) => {
                let token_kind = token.kind();
                let token = token.to_ocaml(context);
                let block = reserve_block(SyntaxKind::Token(token_kind).ocaml_tag(), 1);
                caml_set_field(block, 0, token);
                block
            }
            SyntaxVariantByRef::SyntaxList(l) => {
                let l = to_list(l, context);
                let block = reserve_block(SyntaxKind::SyntaxList.ocaml_tag(), 1);
                caml_set_field(block, 0, l);
                block
            }
            _ => {
                let tag = self.kind().ocaml_tag() as u8;
                // This could be much more readable by constructing a vector of children and
                // passing it to to_list, but the cost of this intermediate vector allocation is
                // too big
                let n = self.iter_children().count();
                let result = reserve_block(tag, n);
                // Similarly, fold() avoids intermediate allocation done by children()
                self.iter_children().fold(0, |i, field| {
                    let field = field.to_ocaml(context);
                    caml_set_field(result, i, field);
                    i + 1
                });
                result
            }
        };
        let block = reserve_block(0, 2);
        caml_set_field(block, 0, syntax);
        caml_set_field(block, 1, value);
        block
    }
}

impl ToOcaml for PositionedTrivium {
    unsafe fn to_ocaml(&self, context: &SerializationContext) -> Value {
        // From full_fidelity_positioned_trivia.ml:
        // type t = {
        //   kind: TriviaKind.t;
        //   source_text : SourceText.t;
        //   offset : int;
        //   width : int
        // }
        let block = reserve_block(0.into(), 4);
        caml_set_field(block, 0, self.kind.to_ocaml(context));
        caml_set_field(block, 1, context.source_text);
        caml_set_field(block, 2, usize_to_ocaml(self.offset));
        caml_set_field(block, 3, usize_to_ocaml(self.width));
        block
    }
}

// TODO (kasper): we replicate LazyTrivia memory saving bit-packing when converting from Rust to
// OCaml values, but Rust values themselves are not packed. We should consider porting this
// optimization there too.
fn trivia_kind_mask(kind: TriviaKind) -> usize {
    1 << (62 - (kind.ocaml_tag()))
}

fn build_lazy_trivia(trivia_list: &[PositionedTrivium], acc: Option<usize>) -> Option<usize> {
    trivia_list
        .iter()
        .fold(acc, |acc, trivia| match (acc, trivia.kind) {
            (None, _) | (_, TriviaKind::ExtraTokenError) => None,
            (Some(mask), kind) => Some(mask | trivia_kind_mask(kind)),
        })
}

impl ToOcaml for PositionedTokenByRef<'_> {
    unsafe fn to_ocaml(&self, context: &SerializationContext) -> Value {
        let kind = self.kind().to_ocaml(context);
        let offset = usize_to_ocaml(self.offset());
        let leading_width = usize_to_ocaml(self.leading_width());
        let width = usize_to_ocaml(self.width());
        let trailing_width = usize_to_ocaml(self.trailing_width());

        let trivia =
            usize_to_ocaml((self.leading_kinds().bits() | self.trailing_kinds().bits()) as usize);
        // From full_fidelity_positioned_token.ml:
        // type t = {
        //   kind: TokenKind.t;
        //   source_text: SourceText.t;
        //   offset: int; (* Beginning of first trivia *)
        //   leading_width: int;
        //   width: int; (* Width of actual token, not counting trivia *)
        //   trailing_width: int;
        //   trivia: LazyTrivia.t;
        // }
        let res = reserve_block(0, 7);
        caml_set_field(res, 0, kind);
        caml_set_field(res, 1, context.source_text);
        caml_set_field(res, 2, offset);
        caml_set_field(res, 3, leading_width);
        caml_set_field(res, 4, width);
        caml_set_field(res, 5, trailing_width);
        caml_set_field(res, 6, trivia);
        res
    }
}

const TOKEN_VALUE_VARIANT: u8 = 0;
const TOKEN_SPAN_VARIANT: u8 = 1;
const MISSING_VALUE_VARIANT: u8 = 2;

impl ToOcaml for PositionedValueByRef<'_> {
    unsafe fn to_ocaml(&self, context: &SerializationContext) -> Value {
        match self {
            PositionedValueByRef::TokenValue(t) => {
                let token = t.to_ocaml(context);
                // TokenValue of  ...
                let block = reserve_block(TOKEN_VALUE_VARIANT, 1);
                caml_set_field(block, 0, token);
                block
            }
            PositionedValueByRef::TokenSpan(left, right) => {
                let left = left.to_ocaml(context);
                let right = right.to_ocaml(context);
                // TokenSpan { left: Token.t; right: Token.t }
                let block = reserve_block(TOKEN_SPAN_VARIANT, 2);
                caml_set_field(block, 0, left);
                caml_set_field(block, 1, right);
                block
            }
            PositionedValueByRef::Missing { offset } => {
                let offset = usize_to_ocaml(*offset);
                // Missing of {...}
                let block = reserve_block(MISSING_VALUE_VARIANT, 2);
                caml_set_field(block, 0, context.source_text);
                caml_set_field(block, 1, offset);
                block
            }
        }
    }
}

impl ToOcaml for SyntaxError {
    unsafe fn to_ocaml(&self, _context: &SerializationContext) -> Value {
        add_to_ambient_pool(self)
    }
}

impl ToOcaml for NoState {
    unsafe fn to_ocaml(&self, _context: &SerializationContext) -> Value {
        add_to_ambient_pool(self)
    }
}

impl ToOcaml for SyntaxKind {
    unsafe fn to_ocaml(&self, _context: &SerializationContext) -> Value {
        add_to_ambient_pool(self)
    }
}

/// Blanket implementation for states of Smart Constructors that need to access SourceText;
/// such SC by convention wrap their state into a pair (State, &SourceText).
impl<'a, T> ToOcaml for (T, SourceText<'a>)
where
    T: ToOcaml,
{
    unsafe fn to_ocaml(&self, _context: &SerializationContext) -> Value {
        // don't serialize .1 (source text) as it is not part the real state we care about
        self.0.to_ocaml(_context)
    }
}

impl ToOcaml for ArenaState<'_> {
    unsafe fn to_ocaml(&self, c: &SerializationContext) -> Value {
        NoState.to_ocaml(c)
    }
}
