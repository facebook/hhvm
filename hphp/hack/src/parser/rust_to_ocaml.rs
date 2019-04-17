/**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*/
extern crate ocaml;
use parser_rust as parser;

use ocaml::core::memory;
use ocaml::core::mlvalues::{empty_list, Size, Tag, Value};

use std::iter::Iterator;

use parser::lexable_token::LexableToken;
use parser::minimal_syntax::MinimalValue;
use parser::minimal_token::MinimalToken;
use parser::minimal_trivia::MinimalTrivia;
use parser::syntax::SyntaxVariant;
use parser::syntax::{Syntax, SyntaxValueType};
use parser::syntax_kind::SyntaxKind;
use parser::syntax_type::SyntaxType;
use parser::token_kind::TokenKind;
use parser::trivia_kind::TriviaKind;

extern "C" {
    fn ocamlpool_reserve_block(tag: Tag, size: Size) -> Value;
}

/* Unsafe functions in this file should be called only:
 * - while being called from OCaml process
 * - between ocamlpool_enter / ocamlpool_leave invocations */
unsafe fn caml_block(tag: Tag, fields: &[Value]) -> Value {
    let result = ocamlpool_reserve_block(tag, fields.len());
    for (i, field) in fields.iter().enumerate() {
        memory::store_field(result, i, *field);
    }
    return result;
}

pub unsafe fn caml_tuple(fields: &[Value]) -> Value {
    caml_block(0, fields)
}

pub struct SerializationContext {}

impl SerializationContext {
    pub fn new() -> Self {
        Self {}
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
        res = caml_tuple(&[v.to_ocaml(context), res])
    }
    res
}

/* Not implementing ToOcaml for integer types, because Value itself is an integer too and it makes
 * it too easy to accidentally treat a pointer to heap as integer and try double convert it */
fn usize_to_ocaml(x: usize) -> Value {
    (x << 1) + 1
}

fn u8_to_ocaml(x: u8) -> Value {
    usize_to_ocaml(x as usize)
}

impl ToOcaml for TokenKind {
    unsafe fn to_ocaml(&self, _context: &SerializationContext) -> Value {
        u8_to_ocaml(self.ocaml_tag())
    }
}

impl ToOcaml for TriviaKind {
    unsafe fn to_ocaml(&self, _context: &SerializationContext) -> Value {
        u8_to_ocaml(self.ocaml_tag())
    }
}

impl ToOcaml for MinimalTrivia {
    unsafe fn to_ocaml(&self, context: &SerializationContext) -> Value {
        /* From full_fidelity_minimal_trivia.ml:
        type t = {
          kind: Full_fidelity_trivia_kind.t;
          width: int
        } */
        let kind = self.kind.to_ocaml(context);
        let width = usize_to_ocaml(self.width);
        caml_tuple(&[kind, width])
    }
}

impl ToOcaml for MinimalToken {
    unsafe fn to_ocaml(&self, context: &SerializationContext) -> Value {
        let kind = self.kind().to_ocaml(context);
        let width = usize_to_ocaml(self.width());
        let leading = to_list(&self.leading, context);
        let trailing = to_list(&self.trailing, context);

        /* From full_fidelity_minimal_token.ml:
        type t = {
          kind: TokenKind.t;
          width: int;
          leading: Trivia.t list;
          trailing: Trivia.t list
        } */
        caml_tuple(&[kind, width, leading, trailing])
    }
}

impl ToOcaml for MinimalValue {
    unsafe fn to_ocaml(&self, _context: &SerializationContext) -> Value {
        /* From full_fidelity_minimal_syntax.ml:
         * type t = { full_width: int } */
        let full_width = usize_to_ocaml(self.full_width);
        caml_tuple(&[full_width])
    }
}

impl<Token, SyntaxValue> ToOcaml for Syntax<Token, SyntaxValue>
where
    Token: LexableToken + ToOcaml,
    SyntaxValue: SyntaxValueType<Token> + ToOcaml,
{
    unsafe fn to_ocaml(&self, context: &SerializationContext) -> Value {
        let value = self.value.to_ocaml(context);

        let syntax = match &self.syntax {
            SyntaxVariant::Missing => u8_to_ocaml(SyntaxKind::Missing.ocaml_tag()),
            SyntaxVariant::Token(token) => {
                let token = token.to_ocaml(context);
                caml_block(SyntaxKind::Token.ocaml_tag(), &[token])
            }
            SyntaxVariant::SyntaxList(l) => {
                let l = to_list(l, context);
                caml_block(SyntaxKind::SyntaxList.ocaml_tag(), &[l])
            }
            _ => {
                let tag = self.kind().ocaml_tag() as u8;
                let children: Vec<Value> = self
                    .children()
                    .iter()
                    .map(|x| x.to_ocaml(context))
                    .collect();
                caml_block(tag, &children)
            }
        };
        caml_tuple(&[syntax, value])
    }
}
