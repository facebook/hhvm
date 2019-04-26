/**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*/
use crate::minimal_smart_constructors::MinimalSmartConstructors;
use crate::parser::Parser;
use crate::smart_constructors::NoState;
use crate::smart_constructors_wrappers::WithKind;

pub type MinimalSyntaxParser<'a> = Parser<'a, WithKind<MinimalSmartConstructors>, NoState>;
