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

            impl<'arena> $crate::DeserializeInArena<'arena> for &'arena $ty
            {
                fn deserialize_in_arena<D>(arena: &'arena Bump, deserializer: D) -> Result<Self, D::Error>
                where
                    D: $crate::serde::Deserializer<'arena>,
                {
                    let value = <$ty>::deserialize(deserializer)?;
                    Ok(arena.alloc(value))
                }
            }
        )+
    };
}

deserialize_without_arena! {
    (),
    bool,
    i8, i16, i32, i64, isize,
    u8, u16, u32, u64, usize,
    f32, f64,
    char, String,
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

impl<'arena, T> DeserializeInArena<'arena> for Option<T>
where
    T: DeserializeInArena<'arena>,
{
    fn deserialize_in_arena<D>(arena: &'arena Bump, deserializer: D) -> Result<Self, D::Error>
    where
        D: Deserializer<'arena>,
    {
        struct OptionVisitor<'arena, T> {
            arena: &'arena Bump,
            marker: PhantomData<fn() -> T>,
        }

        impl<'arena, T: DeserializeInArena<'arena>> Visitor<'arena> for OptionVisitor<'arena, T> {
            type Value = Option<T>;

            fn expecting(&self, formatter: &mut fmt::Formatter) -> fmt::Result {
                formatter.write_str("option")
            }

            #[inline]
            fn visit_unit<E>(self) -> Result<Self::Value, E>
            where
                E: serde::de::Error,
            {
                Ok(None)
            }

            #[inline]
            fn visit_none<E>(self) -> Result<Self::Value, E>
            where
                E: serde::de::Error,
            {
                Ok(None)
            }

            #[inline]
            fn visit_some<D>(self, deserializer: D) -> Result<Self::Value, D::Error>
            where
                D: Deserializer<'arena>,
            {
                T::deserialize_in_arena(self.arena, deserializer).map(Some)
            }
        }

        deserializer.deserialize_option(OptionVisitor {
            arena,
            marker: PhantomData,
        })
    }
}

macro_rules! tuple_impls {
    ($($len:tt => ($($n:tt $name:ident)+))+) => {
        $(
            impl<'arena, $($name: DeserializeInArena<'arena>),+> DeserializeInArena<'arena> for ($($name,)+) {
                #[inline]
                fn deserialize_in_arena<D>(_arena: &'arena Bump, deserializer: D) -> Result<Self, D::Error>
                where
                    D: Deserializer<'arena>,
                {
                    struct TupleVisitor<$($name,)+> {
                        marker: PhantomData<($($name,)+)>,
                    }

                    impl<'arena, $($name: DeserializeInArena<'arena>),+> Visitor<'arena> for TupleVisitor<$($name,)+> {
                        type Value = ($($name,)+);

                        fn expecting(&self, formatter: &mut fmt::Formatter) -> fmt::Result {
                            formatter.write_str(concat!("a tuple of size ", $len))
                        }

                        #[inline]
                        #[allow(non_snake_case)]
                        fn visit_seq<A>(self, mut seq: A) -> Result<Self::Value, A::Error>
                        where
                            A: SeqAccess<'arena>,
                        {
                            $(
                                let seed = ArenaSeed::new();
                                let $name = match seq.next_element_seed(seed)? {
                                    Some(value) => value,
                                    None => return Err(serde::de::Error::invalid_length($n, &self)),
                                };
                            )+

                            Ok(($($name,)+))
                        }
                    }

                    deserializer.deserialize_tuple($len, TupleVisitor { marker: PhantomData })
                }
            }

            impl<'arena, $($name: DeserializeInArena<'arena>),+> DeserializeInArena<'arena> for &'arena ($($name,)+) {
                #[inline]
                fn deserialize_in_arena<D>(arena: &'arena Bump, deserializer: D) -> Result<Self, D::Error>
                where
                    D: Deserializer<'arena>,
                {
                    let v = <($($name,)+)>::deserialize_in_arena(arena, deserializer)?;
                    Ok(arena.alloc(v))
                }
            }
        )+
    }
}

tuple_impls! {
    1  => (0 T0)
    2  => (0 T0 1 T1)
    3  => (0 T0 1 T1 2 T2)
    4  => (0 T0 1 T1 2 T2 3 T3)
    5  => (0 T0 1 T1 2 T2 3 T3 4 T4)
    6  => (0 T0 1 T1 2 T2 3 T3 4 T4 5 T5)
    7  => (0 T0 1 T1 2 T2 3 T3 4 T4 5 T5 6 T6)
    8  => (0 T0 1 T1 2 T2 3 T3 4 T4 5 T5 6 T6 7 T7)
    9  => (0 T0 1 T1 2 T2 3 T3 4 T4 5 T5 6 T6 7 T7 8 T8)
    10 => (0 T0 1 T1 2 T2 3 T3 4 T4 5 T5 6 T6 7 T7 8 T8 9 T9)
    11 => (0 T0 1 T1 2 T2 3 T3 4 T4 5 T5 6 T6 7 T7 8 T8 9 T9 10 T10)
    12 => (0 T0 1 T1 2 T2 3 T3 4 T4 5 T5 6 T6 7 T7 8 T8 9 T9 10 T10 11 T11)
    13 => (0 T0 1 T1 2 T2 3 T3 4 T4 5 T5 6 T6 7 T7 8 T8 9 T9 10 T10 11 T11 12 T12)
    14 => (0 T0 1 T1 2 T2 3 T3 4 T4 5 T5 6 T6 7 T7 8 T8 9 T9 10 T10 11 T11 12 T12 13 T13)
    15 => (0 T0 1 T1 2 T2 3 T3 4 T4 5 T5 6 T6 7 T7 8 T8 9 T9 10 T10 11 T11 12 T12 13 T13 14 T14)
    16 => (0 T0 1 T1 2 T2 3 T3 4 T4 5 T5 6 T6 7 T7 8 T8 9 T9 10 T10 11 T11 12 T12 13 T13 14 T14 15 T15)
}
