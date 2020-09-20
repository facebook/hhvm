// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub mod map {
    pub use crate::typing_cont_key::*;
    pub type Map<'a, TV> = arena_collections::map::Map<'a, TypingContKey<'a>, TV>;
}
