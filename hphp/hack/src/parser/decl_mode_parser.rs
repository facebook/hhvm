// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod decl_mode_smart_constructors;
mod decl_mode_smart_constructors_generated;

use crate::decl_mode_smart_constructors::{DeclModeSmartConstructors, State as DeclModeState};
use ocaml::core::mlvalues::Value;
use parser_rust::{
    parser::Parser,
    positioned_syntax::{PositionedSyntax, PositionedValue},
    positioned_token::PositionedToken,
    smart_constructors_wrappers::WithKind,
};
use rust_to_ocaml::{SerializationContext, ToOcaml};

pub type SmartConstructors<'a> =
    WithKind<DeclModeSmartConstructors<'a, PositionedSyntax, PositionedToken, PositionedValue>>;

pub type ScState<'a> = DeclModeState<'a, PositionedSyntax>;

pub type DeclModeParser<'a> = Parser<'a, SmartConstructors<'a>, ScState<'a>>;

impl<S> ToOcaml for DeclModeState<'_, S> {
    unsafe fn to_ocaml(&self, context: &SerializationContext) -> Value {
        self.stack().to_ocaml(context)
    }
}
