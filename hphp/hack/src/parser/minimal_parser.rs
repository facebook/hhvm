/**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*/
mod minimal_smart_constructors;

use crate::minimal_smart_constructors::MinimalSmartConstructors;
use parser_rust::{
    parser::Parser, smart_constructors::NoState, smart_constructors_wrappers::WithKind,
};

pub type SmartConstructors = WithKind<MinimalSmartConstructors>;
pub type ScState = NoState;
pub type MinimalSyntaxParser<'a> = Parser<'a, SmartConstructors, ScState>;
