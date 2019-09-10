// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod arena;
mod block;
mod impls;
mod value;

pub use arena::Arena;
pub use block::{Block, BlockBuilder};
pub use value::Value;

pub trait OcamlRep {
    fn into_ocamlrep<'a>(self, arena: &Arena<'a>) -> Value<'a>;
}
