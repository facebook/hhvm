// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::rust_to_ocaml::*;
use ocaml::core::mlvalues::Value as OcamlValue;
use parser::lexable_token::LexableToken;
use parser::positioned_token::PositionedToken;
use parser::syntax::{SyntaxTypeBase, SyntaxValueType};
use parser::syntax_kind::SyntaxKind;
use parser_rust as parser;

use ocamlpool_rust::utils::*;

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
            caml_tuple(&[caml_block(kind.ocaml_tag(), children), ocaml_value])
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
            Self {
                syntax: caml_tuple(&[kind, ocaml_value]),
                value,
            }
        }
    }

    fn make_token(ctx: &C, arg: Self::Token) -> Self {
        unsafe {
            let value = V::from_token(&arg);
            let ocaml_value = value.to_ocaml(ctx.serialization_context());
            let syntax = caml_tuple(&[
                caml_block(
                    SyntaxKind::Token(arg.kind()).ocaml_tag(),
                    &[arg.to_ocaml(ctx.serialization_context())],
                ),
                ocaml_value,
            ]);
            Self { syntax, value }
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
                let syntax = caml_tuple(&[
                    caml_block(
                        SyntaxKind::SyntaxList.ocaml_tag(),
                        &[to_list(&args, ctx.serialization_context())],
                    ),
                    ocaml_value,
                ]);
                Self { syntax, value }
            }
        }
    }

    fn value(&self) -> &Self::Value {
        &self.value
    }
}
