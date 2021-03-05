// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep::{ptr::UnsafeOcamlPtr, Allocator, OpaqueValue, ToOcamlRep};
use parser_core_types::{
    lexable_token::LexableToken,
    positioned_trivia::PositionedTrivium,
    syntax_by_ref::{
        positioned_token::PositionedToken, positioned_value::PositionedValue, syntax::Syntax,
        syntax_variant_generated::SyntaxVariant,
    },
    syntax_kind::SyntaxKind,
};

pub struct WithContext<'a, T: ?Sized> {
    pub t: &'a T,
    pub source_text: UnsafeOcamlPtr,
}

impl<T: ?Sized> WithContext<'_, T> {
    pub fn with<'a, S: ?Sized>(&self, s: &'a S) -> WithContext<'a, S> {
        WithContext {
            t: s,
            source_text: self.source_text,
        }
    }
}

fn from_list<'a, 'r, A: Allocator, T>(l: &WithContext<'r, [T]>, alloc: &'a A) -> OpaqueValue<'a>
where
    WithContext<'r, T>: ToOcamlRep,
{
    let mut hd = alloc.add(&());
    for val in l.t.iter().rev() {
        let mut block = alloc.block_with_size(2);
        alloc.set_field(&mut block, 0, l.with(val).to_ocamlrep(alloc));
        alloc.set_field(&mut block, 1, hd);
        hd = block.build();
    }
    hd
}

impl<'r> ToOcamlRep for WithContext<'r, Syntax<'_, PositionedToken<'_>, PositionedValue<'_>>> {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> OpaqueValue<'a> {
        let value = self.with(&self.t.value).to_ocamlrep(alloc);

        let syntax = match &self.t.children {
            SyntaxVariant::Missing => (SyntaxKind::Missing.ocaml_tag() as usize).to_ocamlrep(alloc),
            SyntaxVariant::Token(t) => {
                let kind = t.kind();
                let t = self.with(t).to_ocamlrep(alloc);
                let mut block =
                    alloc.block_with_size_and_tag(1, SyntaxKind::Token(kind).ocaml_tag());
                alloc.set_field(&mut block, 0, t);
                block.build()
            }
            SyntaxVariant::SyntaxList(l) => {
                let l = from_list(&self.with(*l), alloc);
                let mut block =
                    alloc.block_with_size_and_tag(1, SyntaxKind::SyntaxList.ocaml_tag());
                alloc.set_field(&mut block, 0, l);
                block.build()
            }
            _ => {
                let tag = self.t.kind().ocaml_tag();
                let n = self.t.iter_children().count();
                let mut block = alloc.block_with_size_and_tag(n, tag);
                self.t.iter_children().fold(0, |i, field| {
                    let field = self.with(field).to_ocamlrep(alloc);
                    alloc.set_field(&mut block, i, field);
                    i + 1
                });
                block.build()
            }
        };
        let mut block = alloc.block_with_size_and_tag(2, 0);
        alloc.set_field(&mut block, 0, syntax);
        alloc.set_field(&mut block, 1, value);
        block.build()
    }
}

impl<'r> ToOcamlRep for WithContext<'r, PositionedTrivium> {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> OpaqueValue<'a> {
        // From full_fidelity_positioned_trivia.ml:
        // type t = {
        //   kind: TriviaKind.t;
        //   source_text : SourceText.t;
        //   offset : int;
        //   width : int
        // }
        let mut block = alloc.block_with_size_and_tag(4, 0);
        alloc.set_field(&mut block, 0, self.t.kind.to_ocamlrep(alloc));
        alloc.set_field(&mut block, 1, self.source_text.to_ocamlrep(alloc));
        alloc.set_field(&mut block, 2, self.t.offset.to_ocamlrep(alloc));
        alloc.set_field(&mut block, 3, self.t.width.to_ocamlrep(alloc));
        block.build()
    }
}

impl<'r> ToOcamlRep for WithContext<'r, PositionedToken<'r>> {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> OpaqueValue<'a> {
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
        let mut block = alloc.block_with_size_and_tag(7, 0);
        alloc.set_field(&mut block, 0, self.t.kind().to_ocamlrep(alloc));
        alloc.set_field(&mut block, 1, self.source_text.to_ocamlrep(alloc));
        alloc.set_field(&mut block, 2, self.t.offset().to_ocamlrep(alloc));
        alloc.set_field(&mut block, 3, self.t.leading_width().to_ocamlrep(alloc));
        alloc.set_field(&mut block, 4, self.t.width().to_ocamlrep(alloc));
        alloc.set_field(&mut block, 5, self.t.trailing_width().to_ocamlrep(alloc));
        alloc.set_field(
            &mut block,
            6,
            ((self.t.leading_kinds().bits() | self.t.trailing_kinds().bits()) as usize)
                .to_ocamlrep(alloc),
        );
        block.build()
    }
}

const TOKEN_VALUE_VARIANT: u8 = 0;
const TOKEN_SPAN_VARIANT: u8 = 1;
const MISSING_VALUE_VARIANT: u8 = 2;

impl<'r> ToOcamlRep for WithContext<'r, PositionedValue<'r>> {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> OpaqueValue<'a> {
        match self.t {
            PositionedValue::TokenValue(t) => {
                let mut block = alloc.block_with_size_and_tag(1, TOKEN_VALUE_VARIANT);
                alloc.set_field(&mut block, 0, self.with(t).to_ocamlrep(alloc));
                block.build()
            }
            PositionedValue::TokenSpan(l, r) => {
                let mut block = alloc.block_with_size_and_tag(2, TOKEN_SPAN_VARIANT);
                alloc.set_field(&mut block, 0, self.with(l).to_ocamlrep(alloc));
                alloc.set_field(&mut block, 1, self.with(r).to_ocamlrep(alloc));
                block.build()
            }
            PositionedValue::Missing { offset } => {
                let mut block = alloc.block_with_size_and_tag(2, MISSING_VALUE_VARIANT);
                alloc.set_field(&mut block, 0, self.source_text.to_ocamlrep(alloc));
                alloc.set_field(&mut block, 1, offset.to_ocamlrep(alloc));
                block.build()
            }
        }
    }
}
