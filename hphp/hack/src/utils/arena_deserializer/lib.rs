// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod deserializer;
mod impls;
mod seed;

use serde::de::{DeserializeSeed, Deserializer};

pub extern crate bumpalo;
pub extern crate serde;
pub use crate::deserializer::ArenaDeserializer;
pub use crate::impls::DeserializeInArena;
pub use crate::seed::ArenaSeed;

pub fn arena<'arena, D, T>(deserializer: D) -> Result<T, D::Error>
where
    D: Deserializer<'arena>,
    T: DeserializeInArena<'arena>,
{
    ArenaSeed::new().deserialize(deserializer)
}
