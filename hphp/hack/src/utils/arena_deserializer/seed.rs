// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::deserializer::obtain_arena;
use crate::impls::DeserializeInArena;
use serde::de::{DeserializeSeed, Deserializer};
use std::marker::PhantomData;

pub struct ArenaSeed<T> {
    marker: PhantomData<fn() -> T>,
}

impl<T> ArenaSeed<T> {
    pub fn new() -> Self {
        let marker = PhantomData;
        ArenaSeed { marker }
    }
}

impl<T> Copy for ArenaSeed<T> {}

impl<T> Clone for ArenaSeed<T> {
    fn clone(&self) -> Self {
        *self
    }
}

impl<'arena, T> DeserializeSeed<'arena> for ArenaSeed<T>
where
    T: DeserializeInArena<'arena>,
{
    type Value = T;

    fn deserialize<D>(self, deserializer: D) -> Result<Self::Value, D::Error>
    where
        D: Deserializer<'arena>,
    {
        let arena = obtain_arena(&deserializer)?;
        T::deserialize_in_arena(arena, deserializer)
    }
}
