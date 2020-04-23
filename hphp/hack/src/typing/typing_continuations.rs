// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use typing_collections_rust::Map;

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub enum TypingContKey<'a> {
    Next,
    Continue,
    Break,
    Catch,
    Do,
    Exit,
    Fallthrough,
    Finally,
    Goto(&'a str),
}

pub type CMap<'a, V> = Map<'a, TypingContKey<'a>, V>;
