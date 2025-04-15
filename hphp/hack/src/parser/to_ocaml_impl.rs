// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep::Allocator;
use ocamlrep::ToOcamlRep;
use ocamlrep::Value;
use ocamlrep::ptr::UnsafeOcamlPtr;
use parser_core_types::lexable_token::LexableToken;
use parser_core_types::positioned_trivia::PositionedTrivium;
use parser_core_types::syntax_by_ref::positioned_token::PositionedToken;
use parser_core_types::syntax_by_ref::positioned_value::PositionedValue;
use parser_core_types::syntax_by_ref::syntax::Syntax;
use parser_core_types::syntax_by_ref::syntax_variant_generated::SyntaxVariant;
use parser_core_types::syntax_kind::SyntaxKind;

pub struct WithContext<'a, T: ?Sized> {
    pub t: &'a T,
    pub source_text: UnsafeOcamlPtr,
}

pub trait ToOcaml {
    fn to_ocaml<'a, A: Allocator>(&'a self, alloc: &'a A, source_text: UnsafeOcamlPtr)
    -> Value<'a>;
}

impl<T: ToOcaml> ToOcaml for [T] {
    fn to_ocaml<'a, A: Allocator>(
        &'a self,
        alloc: &'a A,
        source_text: UnsafeOcamlPtr,
    ) -> Value<'a> {
        let mut hd = alloc.add(&());
        for val in self.iter().rev() {
            let mut block = alloc.block_with_size(2);
            alloc.set_field(&mut block, 0, val.to_ocaml(alloc, source_text));
            alloc.set_field(&mut block, 1, hd);
            hd = block.build();
        }
        hd
    }
}

impl ToOcaml for Syntax<'_, PositionedToken<'_>, PositionedValue<'_>> {
    fn to_ocaml<'a, A: Allocator>(
        &'a self,
        alloc: &'a A,
        source_text: UnsafeOcamlPtr,
    ) -> Value<'a> {
        let value = self.value.to_ocaml(alloc, source_text);

        let syntax = match &self.children {
            SyntaxVariant::Missing => alloc.add_copy(SyntaxKind::Missing.ocaml_tag() as usize),
            SyntaxVariant::Token(t) => {
                let kind = t.kind();
                let t = t.to_ocaml(alloc, source_text);
                let mut block =
                    alloc.block_with_size_and_tag(1, SyntaxKind::Token(kind).ocaml_tag());
                alloc.set_field(&mut block, 0, t);
                block.build()
            }
            SyntaxVariant::SyntaxList(l) => {
                let l = l.to_ocaml(alloc, source_text);
                let mut block =
                    alloc.block_with_size_and_tag(1, SyntaxKind::SyntaxList.ocaml_tag());
                alloc.set_field(&mut block, 0, l);
                block.build()
            }
            _ => {
                // TODO: rewrite this iteratively.
                let tag = self.kind().ocaml_tag();
                let n = self.iter_children().count();
                let mut block = alloc.block_with_size_and_tag(n, tag);
                stack_limit::maybe_grow(|| {
                    for (i, field) in self.iter_children().enumerate() {
                        let field = field.to_ocaml(alloc, source_text);
                        alloc.set_field(&mut block, i, field);
                    }
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

impl ToOcaml for PositionedTrivium {
    fn to_ocaml<'a, A: Allocator>(
        &'a self,
        alloc: &'a A,
        source_text: UnsafeOcamlPtr,
    ) -> Value<'a> {
        // From full_fidelity_positioned_trivia.ml:
        // type t = {
        //   kind: TriviaKind.t;
        //   source_text : SourceText.t;
        //   offset : int;
        //   width : int
        // }
        let mut block = alloc.block_with_size_and_tag(4, 0);
        alloc.set_field(&mut block, 0, self.kind.to_ocamlrep(alloc));
        alloc.set_field(&mut block, 1, alloc.add_copy(source_text));
        alloc.set_field(&mut block, 2, self.offset.to_ocamlrep(alloc));
        alloc.set_field(&mut block, 3, self.width.to_ocamlrep(alloc));
        block.build()
    }
}

impl ToOcaml for PositionedToken<'_> {
    fn to_ocaml<'a, A: Allocator>(
        &'a self,
        alloc: &'a A,
        source_text: UnsafeOcamlPtr,
    ) -> Value<'a> {
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
        alloc.set_field(&mut block, 0, alloc.add_copy(self.kind()));
        alloc.set_field(&mut block, 1, alloc.add_copy(source_text));
        alloc.set_field(&mut block, 2, alloc.add_copy(self.offset()));
        alloc.set_field(&mut block, 3, alloc.add_copy(self.leading_width()));
        alloc.set_field(&mut block, 4, alloc.add_copy(self.width()));
        alloc.set_field(&mut block, 5, alloc.add_copy(self.trailing_width()));
        alloc.set_field(
            &mut block,
            6,
            alloc.add_copy((self.leading_kinds().bits() | self.trailing_kinds().bits()) as usize),
        );
        block.build()
    }
}

const TOKEN_VALUE_VARIANT: u8 = 0;
const TOKEN_SPAN_VARIANT: u8 = 1;
const MISSING_VALUE_VARIANT: u8 = 2;

impl ToOcaml for PositionedValue<'_> {
    fn to_ocaml<'a, A: Allocator>(
        &'a self,
        alloc: &'a A,
        source_text: UnsafeOcamlPtr,
    ) -> Value<'a> {
        match self {
            PositionedValue::TokenValue(t) => {
                let mut block = alloc.block_with_size_and_tag(1, TOKEN_VALUE_VARIANT);
                alloc.set_field(&mut block, 0, t.to_ocaml(alloc, source_text));
                block.build()
            }
            PositionedValue::TokenSpan(l, r) => {
                let mut block = alloc.block_with_size_and_tag(2, TOKEN_SPAN_VARIANT);
                alloc.set_field(&mut block, 0, l.to_ocaml(alloc, source_text));
                alloc.set_field(&mut block, 1, r.to_ocaml(alloc, source_text));
                block.build()
            }
            PositionedValue::Missing { offset } => {
                let mut block = alloc.block_with_size_and_tag(2, MISSING_VALUE_VARIANT);
                alloc.set_field(&mut block, 0, alloc.add_copy(source_text));
                alloc.set_field(&mut block, 1, offset.to_ocamlrep(alloc));
                block.build()
            }
        }
    }
}
