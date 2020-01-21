// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod verify_smart_constructors;
mod verify_smart_constructors_generated;

use crate::verify_smart_constructors::{State as VerifyState, VerifySmartConstructors};
use ocaml::core::mlvalues::Value;
use parser::{parser::Parser, smart_constructors_wrappers::WithKind};
use rust_to_ocaml::{to_list, SerializationContext, ToOcaml};

pub type SmartConstructors = WithKind<VerifySmartConstructors>;

pub type ScState = VerifyState;

pub type VerifyParser<'a> = Parser<'a, SmartConstructors, ScState>;

impl ToOcaml for VerifyState {
    unsafe fn to_ocaml(&self, context: &SerializationContext) -> Value {
        to_list(self.stack(), context)
    }
}
