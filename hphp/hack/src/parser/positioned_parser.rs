// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub mod positioned_smart_constructors;

use parser::{parser::Parser, smart_constructors::NoState, smart_constructors_wrappers::WithKind};
use positioned_smart_constructors::*;

pub type SmartConstructors = WithKind<PositionedSmartConstructors>;
pub type ScState = NoState;
pub type PositionedSyntaxParser<'a> = Parser<'a, SmartConstructors, ScState>;
