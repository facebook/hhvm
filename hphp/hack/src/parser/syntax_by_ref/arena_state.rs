// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use super::has_arena::HasArena;
use bumpalo::Bump;

#[derive(Clone)]
pub struct State<'a> {
    pub arena: &'a Bump,
}

impl<'a> HasArena<'a> for State<'a> {
    fn get_arena(&self) -> &'a Bump {
        self.arena
    }
}
