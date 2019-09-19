// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use coroutine_smart_constructors::{CoroutineSmartConstructors, State as CoroutineState};
use parser_rust::{
    parser::Parser, positioned_syntax::PositionedSyntax, smart_constructors_wrappers::WithKind,
};

pub type CoroutineParserLeakTree<'a> = Parser<
    'a,
    WithKind<
        CoroutineSmartConstructors<'a, PositionedSyntax, CoroutineState<'a, PositionedSyntax>>,
    >,
    CoroutineState<'a, PositionedSyntax>,
>;
