// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fmt;
use std::marker::PhantomData;

use bumpalo::collections::Vec as ArenaVec;
use bumpalo::Bump;
use ocamlrep_caml_builtins::Int64;
use serde::de::Deserializer;
use serde::de::SeqAccess;
use serde::de::Visitor;
use serde::Deserialize;

use crate::seed::ArenaSeed;

pub trait DeserializeInArena<'arena>: Sized {
    fn deserialize_in_arena<D>(arena: &'arena Bump, deserializer: D) -> Result<Self, D::Error>
    where
        D: Deserializer<'arena>;
}

#[macro_export]
macro_rules! impl_deserialize_in_arena{
    ($ty:ident < $($lt:lifetime),* $(,)? $($tp:ident),* >) => {
            impl<'arena, $($tp : 'arena + $crate::DeserializeInArena<'arena>),*> $crate::DeserializeInArena<'arena> for $ty<$($lt,)* $($tp,)* > {
            fn deserialize_in_arena<__D>(
                arena: &'arena $crate::bumpalo::Bump,
                deserializer: __D,
            ) -> Result<Self, __D::Error>
            where
                __D: $crate::serde::Deserializer<'arena>,
            {
                let _ = arena;
                $crate::serde::Deserialize::deserialize(deserializer)
            }
        }

        impl<'arena, $($tp : $crate::DeserializeInArena<'arena>),*> $crate::DeserializeInArena<'arena> for &'arena $ty<$($lt,)* $($tp,)* > {
            fn deserialize_in_arena<__D>(
                arena: &'arena $crate::bumpalo::Bump,
                deserializer: __D,
            ) -> Result<Self, __D::Error>
            where
                __D: $crate::serde::Deserializer<'arena>,
            {
                let value = <$ty<$($tp,)*>>::deserialize_in_arena(arena, deserializer)?;
                Ok(arena.alloc(value))
            }
        }
    };
    ($ty:ty) => {
        impl<'arena> $crate::DeserializeInArena<'arena> for $ty {
            fn deserialize_in_arena<__D>(
                arena: &'arena $crate::bumpalo::Bump,
                deserializer: __D,
            ) -> Result<Self, __D::Error>
            where
                __D: $crate::serde::Deserializer<'arena>,
            {
                let _ = arena;
                $crate::serde::Deserialize::deserialize(deserializer)
            }
        }

        impl<'arena> $crate::DeserializeInArena<'arena> for &'arena $ty {
            fn deserialize_in_arena<__D>(
                arena: &'arena $crate::bumpalo::Bump,
                deserializer: __D,
            ) -> Result<Self, __D::Error>
            where
                __D: $crate::serde::Deserializer<'arena>,
            {
                let value = <$ty>::deserialize(deserializer)?;
                Ok(arena.alloc(value))
            }
        }
    };
}

impl_deserialize_in_arena!(());
impl_deserialize_in_arena!(bool);
impl_deserialize_in_arena!(i8);
impl_deserialize_in_arena!(i16);
impl_deserialize_in_arena!(i32);
impl_deserialize_in_arena!(i64);
impl_deserialize_in_arena!(isize);
impl_deserialize_in_arena!(u8);
impl_deserialize_in_arena!(u16);
impl_deserialize_in_arena!(u32);
impl_deserialize_in_arena!(u64);
impl_deserialize_in_arena!(usize);
impl_deserialize_in_arena!(f32);
impl_deserialize_in_arena!(f64);
impl_deserialize_in_arena!(char);
impl_deserialize_in_arena!(Int64);

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

            fn expecting(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {
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

impl<'arena> DeserializeInArena<'arena> for &'arena bstr::BStr {
    fn deserialize_in_arena<D>(arena: &'arena Bump, deserializer: D) -> Result<Self, D::Error>
    where
        D: Deserializer<'arena>,
    {
        struct BStrVisitor<'arena> {
            arena: &'arena Bump,
        }

        impl<'arena, 'de> Visitor<'de> for BStrVisitor<'arena> {
            type Value = &'arena bstr::BStr;

            fn expecting(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
                f.write_str("a borrowed byte string")
            }

            #[inline]
            fn visit_seq<V: SeqAccess<'de>>(self, mut visitor: V) -> Result<Self::Value, V::Error> {
                let len = std::cmp::min(visitor.size_hint().unwrap_or(0), 256);
                let mut bytes = ArenaVec::with_capacity_in(len, self.arena);
                while let Some(v) = visitor.next_element()? {
                    bytes.push(v);
                }
                Ok(bytes.into_bump_slice().into())
            }

            #[inline]
            fn visit_borrowed_bytes<E>(self, value: &'de [u8]) -> Result<Self::Value, E>
            where
                E: serde::de::Error,
            {
                Ok((self.arena.alloc_slice_copy(value) as &[u8]).into())
            }

            #[inline]
            fn visit_borrowed_str<E>(self, value: &'de str) -> Result<Self::Value, E>
            where
                E: serde::de::Error,
            {
                Ok((self.arena.alloc_str(value) as &str).into())
            }

            #[inline]
            fn visit_bytes<E>(self, value: &[u8]) -> Result<Self::Value, E>
            where
                E: serde::de::Error,
            {
                Ok((self.arena.alloc_slice_copy(value) as &[u8]).into())
            }
        }

        deserializer.deserialize_bytes(BStrVisitor { arena })
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

            fn expecting(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {
                formatter.write_str("an array")
            }

            fn visit_seq<A>(self, mut seq: A) -> Result<Self::Value, A::Error>
            where
                A: SeqAccess<'arena>,
            {
                let mut vec = Vec::with_capacity(seq.size_hint().unwrap_or(0));
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

            fn expecting(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {
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

impl<'arena, T> DeserializeInArena<'arena> for &'arena Option<T>
where
    T: DeserializeInArena<'arena>,
{
    fn deserialize_in_arena<D>(arena: &'arena Bump, deserializer: D) -> Result<Self, D::Error>
    where
        D: Deserializer<'arena>,
    {
        let v = <Option<T>>::deserialize_in_arena(arena, deserializer)?;
        Ok(arena.alloc(v))
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

                        fn expecting(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {
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
