// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod ocaml_coroutine_state;
pub mod ocaml_syntax;
mod ocaml_syntax_generated;

use crate::{ocaml_coroutine_state::OcamlCoroutineState, ocaml_syntax::OcamlSyntax};
use coroutine_smart_constructors::{CoroutineSmartConstructors, CoroutineStateType};
use ocaml::core::mlvalues::Value;
use parser_rust::{
    parser::Parser, positioned_syntax::PositionedValue, smart_constructors::WithKind,
};
use rust_to_ocaml::{SerializationContext, ToOcaml};

pub type CoroutineParser<'a> = Parser<
    'a,
    WithKind<
        CoroutineSmartConstructors<
            'a,
            OcamlSyntax<PositionedValue>,
            OcamlCoroutineState<'a, OcamlSyntax<PositionedValue>>,
        >,
    >,
    OcamlCoroutineState<'a, OcamlSyntax<PositionedValue>>,
>;

impl<'a, S> ToOcaml for OcamlCoroutineState<'a, S> {
    unsafe fn to_ocaml(&self, context: &SerializationContext) -> Value {
        self.seen_ppl().to_ocaml(context)
    }
}

impl<V> ToOcaml for OcamlSyntax<V> {
    unsafe fn to_ocaml(&self, _context: &SerializationContext) -> Value {
        self.syntax
    }
}
