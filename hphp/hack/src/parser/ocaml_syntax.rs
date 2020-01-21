// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub mod ocaml_coroutine_state;
mod ocaml_syntax_generated;

use ocaml::core::mlvalues::Value as OcamlValue;
use ocaml::core::mlvalues::Value;
use ocamlpool_rust::utils::*;
use parser_core_types::{
    lexable_token::LexableToken,
    positioned_token::PositionedToken,
    syntax::{SyntaxTypeBase, SyntaxValueType},
    syntax_kind::SyntaxKind,
};
use rust_to_ocaml::*;

pub use crate::ocaml_syntax_generated::*;

#[derive(Debug, Clone)]
pub struct OcamlSyntax<V> {
    pub syntax: OcamlValue,
    pub value: V,
}

pub trait Context {
    fn serialization_context(&self) -> &SerializationContext;
}

impl<V> OcamlSyntax<V>
where
    V: SyntaxValueType<PositionedToken> + ToOcaml,
{
    pub fn make<C: Context>(
        ctx: &C,
        kind: SyntaxKind,
        value: &V,
        children: &[OcamlValue],
    ) -> OcamlValue {
        unsafe {
            let ocaml_value = value.to_ocaml(ctx.serialization_context());
            let syntax = reserve_block(kind.ocaml_tag().into(), children.len());
            for (i, &child) in children.iter().enumerate() {
                caml_set_field(syntax, i, child);
            }
            let node = reserve_block(0, 2);
            caml_set_field(node, 0, syntax);
            caml_set_field(node, 1, ocaml_value);
            node
        }
    }
}

impl<V, C> SyntaxTypeBase<'_, C> for OcamlSyntax<V>
where
    C: Context,
    V: SyntaxValueType<PositionedToken> + ToOcaml,
{
    type Token = PositionedToken;
    type Value = V;

    fn make_missing(ctx: &C, offset: usize) -> Self {
        unsafe {
            let value = V::from_children(SyntaxKind::Missing, offset, &[]);
            let ocaml_value = value.to_ocaml(ctx.serialization_context());
            let kind = u8_to_ocaml(SyntaxKind::Missing.ocaml_tag());
            let node = reserve_block(0, 2);
            caml_set_field(node, 0, kind);
            caml_set_field(node, 1, ocaml_value);
            Self {
                syntax: node,
                value,
            }
        }
    }

    fn make_token(ctx: &C, arg: Self::Token) -> Self {
        unsafe {
            let value = V::from_token(&arg);
            let ocaml_value = value.to_ocaml(ctx.serialization_context());
            let syntax = reserve_block(SyntaxKind::Token(arg.kind()).ocaml_tag().into(), 1);
            caml_set_field(syntax, 0, arg.to_ocaml(ctx.serialization_context()));
            let node = reserve_block(0.into(), 2);
            caml_set_field(node, 0, syntax);
            caml_set_field(node, 1, ocaml_value);
            Self {
                syntax: node,
                value,
            }
        }
    }

    fn make_list(ctx: &C, args: Vec<Self>, offset: usize) -> Self {
        unsafe {
            if args.is_empty() {
                Self::make_missing(ctx, offset)
            } else {
                // TODO: avoid creating Vec
                let lst_slice = &args.iter().map(|x| &x.value).collect::<Vec<_>>();
                let value = V::from_children(SyntaxKind::SyntaxList, offset, lst_slice);
                let ocaml_value = value.to_ocaml(ctx.serialization_context());
                let syntax = reserve_block(SyntaxKind::SyntaxList.ocaml_tag().into(), 1);
                caml_set_field(syntax, 0, to_list(&args, ctx.serialization_context()));
                let node = reserve_block(0.into(), 2);
                caml_set_field(node, 0, syntax);
                caml_set_field(node, 1, ocaml_value);
                Self {
                    syntax: node,
                    value,
                }
            }
        }
    }

    fn value(&self) -> &Self::Value {
        &self.value
    }
}

impl<V> ToOcaml for OcamlSyntax<V> {
    unsafe fn to_ocaml(&self, _context: &SerializationContext) -> Value {
        self.syntax
    }
}
