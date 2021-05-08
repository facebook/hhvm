// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::seed::ArenaSeed;
use bumpalo::Bump;
use serde::de::{Deserializer, SeqAccess, Visitor};
use serde::Deserialize;
use std::fmt;
use std::marker::PhantomData;

pub trait DeserializeInArena<'arena>: Sized {
    fn deserialize_in_arena<D>(arena: &'arena Bump, deserializer: D) -> Result<Self, D::Error>
    where
        D: Deserializer<'arena>;
}

#[macro_export]
macro_rules! deserialize_without_arena {
    ($($ty:ty),+ $(,)?) => {
        $(
            impl<'arena> $crate::DeserializeInArena<'arena> for $ty {
                fn deserialize_in_arena<D>(
                    arena: &'arena $crate::bumpalo::Bump,
                    deserializer: D,
                ) -> Result<Self, D::Error>
                where
                    D: $crate::serde::Deserializer<'arena>,
                {
                    let _ = arena;
                    $crate::serde::Deserialize::deserialize(deserializer)
                }
            }
        )+
    };
}

deserialize_without_arena! {
    bool,
    i8, i16, i32, i64, isize,
    u8, u16, u32, u64, usize,
    f32, f64,
    char, String,
}

impl<'arena, T> DeserializeInArena<'arena> for &'arena T
where
    T: Deserialize<'arena>,
{
    fn deserialize_in_arena<D>(arena: &'arena Bump, deserializer: D) -> Result<Self, D::Error>
    where
        D: Deserializer<'arena>,
    {
        let value = T::deserialize(deserializer)?;
        Ok(arena.alloc(value))
    }
}

impl<'arena> DeserializeInArena<'arena> for &'arena str {
    fn deserialize_in_arena<D>(arena: &'arena Bump, deserializer: D) -> Result<Self, D::Error>
    where
        D: Deserializer<'arena>,
    {
        struct StrInArena<'arena> {
            arena: &'arena Bump,
        }

        impl<'arena, 'de> Visitor<'de> for StrInArena<'arena> {
            type Value = &'arena str;

            fn expecting(&self, formatter: &mut fmt::Formatter) -> fmt::Result {
                formatter.write_str("a string")
            }

            fn visit_str<E>(self, string: &str) -> Result<Self::Value, E>
            where
                E: serde::de::Error,
            {
                Ok(self.arena.alloc_str(string))
            }
        }

        deserializer.deserialize_str(StrInArena { arena })
    }
}

impl<'arena, T> DeserializeInArena<'arena> for &'arena [T]
where
    T: DeserializeInArena<'arena>,
{
    fn deserialize_in_arena<D>(arena: &'arena Bump, deserializer: D) -> Result<Self, D::Error>
    where
        D: Deserializer<'arena>,
    {
        struct SliceInArena<'arena, T> {
            arena: &'arena Bump,
            marker: PhantomData<fn() -> T>,
        }

        impl<'arena, T> Visitor<'arena> for SliceInArena<'arena, T>
        where
            T: DeserializeInArena<'arena> + 'arena,
        {
            type Value = &'arena [T];

            fn expecting(&self, formatter: &mut fmt::Formatter) -> fmt::Result {
                formatter.write_str("an array")
            }

            fn visit_seq<A>(self, mut seq: A) -> Result<Self::Value, A::Error>
            where
                A: SeqAccess<'arena>,
            {
                let mut vec = Vec::new();
                let seed = ArenaSeed::new();
                while let Some(value) = seq.next_element_seed(seed)? {
                    vec.push(value);
                }
                Ok(self.arena.alloc_slice_fill_iter(vec))
            }
        }

        deserializer.deserialize_seq(SliceInArena {
            arena,
            marker: PhantomData,
        })
    }
}
