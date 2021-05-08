// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bumpalo::Bump;
use serde::de::{
    DeserializeSeed, Deserializer, EnumAccess, MapAccess, SeqAccess, VariantAccess, Visitor,
};
use std::marker::PhantomData;
use std::{any, fmt, mem};

#[repr(C)]
pub struct ArenaDeserializer<'arena, 'de, D> {
    arena: &'arena Bump, // must be first field
    delegate: D,
    marker: PhantomData<fn() -> &'de ()>,
}

impl<'arena, 'de, D> ArenaDeserializer<'arena, 'de, D> {
    pub fn new(arena: &'arena Bump, delegate: D) -> Self {
        ArenaDeserializer {
            arena,
            delegate,
            marker: PhantomData,
        }
    }
}

impl<'arena, 'de, D> Deserializer<'arena> for ArenaDeserializer<'arena, 'de, D>
where
    D: Deserializer<'de>,
{
    type Error = <D as Deserializer<'de>>::Error;

    fn deserialize_any<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'arena>,
    {
        self.delegate.deserialize_any(ArenaDeserializer {
            arena: self.arena,
            delegate: visitor,
            marker: PhantomData,
        })
    }

    fn deserialize_bool<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'arena>,
    {
        self.delegate.deserialize_bool(ArenaDeserializer {
            arena: self.arena,
            delegate: visitor,
            marker: PhantomData,
        })
    }

    fn deserialize_i8<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'arena>,
    {
        self.delegate.deserialize_i8(ArenaDeserializer {
            arena: self.arena,
            delegate: visitor,
            marker: PhantomData,
        })
    }

    fn deserialize_i16<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'arena>,
    {
        self.delegate.deserialize_i16(ArenaDeserializer {
            arena: self.arena,
            delegate: visitor,
            marker: PhantomData,
        })
    }

    fn deserialize_i32<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'arena>,
    {
        self.delegate.deserialize_i32(ArenaDeserializer {
            arena: self.arena,
            delegate: visitor,
            marker: PhantomData,
        })
    }

    fn deserialize_i64<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'arena>,
    {
        self.delegate.deserialize_i64(ArenaDeserializer {
            arena: self.arena,
            delegate: visitor,
            marker: PhantomData,
        })
    }

    fn deserialize_u8<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'arena>,
    {
        self.delegate.deserialize_u8(ArenaDeserializer {
            arena: self.arena,
            delegate: visitor,
            marker: PhantomData,
        })
    }

    fn deserialize_u16<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'arena>,
    {
        self.delegate.deserialize_u16(ArenaDeserializer {
            arena: self.arena,
            delegate: visitor,
            marker: PhantomData,
        })
    }

    fn deserialize_u32<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'arena>,
    {
        self.delegate.deserialize_u32(ArenaDeserializer {
            arena: self.arena,
            delegate: visitor,
            marker: PhantomData,
        })
    }

    fn deserialize_u64<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'arena>,
    {
        self.delegate.deserialize_u64(ArenaDeserializer {
            arena: self.arena,
            delegate: visitor,
            marker: PhantomData,
        })
    }

    fn deserialize_f32<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'arena>,
    {
        self.delegate.deserialize_f32(ArenaDeserializer {
            arena: self.arena,
            delegate: visitor,
            marker: PhantomData,
        })
    }

    fn deserialize_f64<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'arena>,
    {
        self.delegate.deserialize_f64(ArenaDeserializer {
            arena: self.arena,
            delegate: visitor,
            marker: PhantomData,
        })
    }

    fn deserialize_char<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'arena>,
    {
        self.delegate.deserialize_char(ArenaDeserializer {
            arena: self.arena,
            delegate: visitor,
            marker: PhantomData,
        })
    }

    fn deserialize_str<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'arena>,
    {
        self.delegate.deserialize_str(ArenaDeserializer {
            arena: self.arena,
            delegate: visitor,
            marker: PhantomData,
        })
    }

    fn deserialize_string<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'arena>,
    {
        self.delegate.deserialize_string(ArenaDeserializer {
            arena: self.arena,
            delegate: visitor,
            marker: PhantomData,
        })
    }

    fn deserialize_bytes<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'arena>,
    {
        self.delegate.deserialize_bytes(ArenaDeserializer {
            arena: self.arena,
            delegate: visitor,
            marker: PhantomData,
        })
    }

    fn deserialize_byte_buf<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'arena>,
    {
        self.delegate.deserialize_byte_buf(ArenaDeserializer {
            arena: self.arena,
            delegate: visitor,
            marker: PhantomData,
        })
    }

    fn deserialize_option<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'arena>,
    {
        self.delegate.deserialize_option(ArenaDeserializer {
            arena: self.arena,
            delegate: visitor,
            marker: PhantomData,
        })
    }

    fn deserialize_unit<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'arena>,
    {
        self.delegate.deserialize_unit(ArenaDeserializer {
            arena: self.arena,
            delegate: visitor,
            marker: PhantomData,
        })
    }

    fn deserialize_unit_struct<V>(
        self,
        name: &'static str,
        visitor: V,
    ) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'arena>,
    {
        self.delegate.deserialize_unit_struct(
            name,
            ArenaDeserializer {
                arena: self.arena,
                delegate: visitor,
                marker: PhantomData,
            },
        )
    }

    fn deserialize_newtype_struct<V>(
        self,
        name: &'static str,
        visitor: V,
    ) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'arena>,
    {
        self.delegate.deserialize_newtype_struct(
            name,
            ArenaDeserializer {
                arena: self.arena,
                delegate: visitor,
                marker: PhantomData,
            },
        )
    }

    fn deserialize_seq<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'arena>,
    {
        self.delegate.deserialize_seq(ArenaDeserializer {
            arena: self.arena,
            delegate: visitor,
            marker: PhantomData,
        })
    }

    fn deserialize_tuple<V>(self, len: usize, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'arena>,
    {
        self.delegate.deserialize_tuple(
            len,
            ArenaDeserializer {
                arena: self.arena,
                delegate: visitor,
                marker: PhantomData,
            },
        )
    }

    fn deserialize_tuple_struct<V>(
        self,
        name: &'static str,
        len: usize,
        visitor: V,
    ) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'arena>,
    {
        self.delegate.deserialize_tuple_struct(
            name,
            len,
            ArenaDeserializer {
                arena: self.arena,
                delegate: visitor,
                marker: PhantomData,
            },
        )
    }

    fn deserialize_map<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'arena>,
    {
        self.delegate.deserialize_map(ArenaDeserializer {
            arena: self.arena,
            delegate: visitor,
            marker: PhantomData,
        })
    }

    fn deserialize_struct<V>(
        self,
        name: &'static str,
        fields: &'static [&'static str],
        visitor: V,
    ) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'arena>,
    {
        self.delegate.deserialize_struct(
            name,
            fields,
            ArenaDeserializer {
                arena: self.arena,
                delegate: visitor,
                marker: PhantomData,
            },
        )
    }

    fn deserialize_enum<V>(
        self,
        name: &'static str,
        variants: &'static [&'static str],
        visitor: V,
    ) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'arena>,
    {
        self.delegate.deserialize_enum(
            name,
            variants,
            ArenaDeserializer {
                arena: self.arena,
                delegate: visitor,
                marker: PhantomData,
            },
        )
    }

    fn deserialize_identifier<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'arena>,
    {
        self.delegate.deserialize_identifier(ArenaDeserializer {
            arena: self.arena,
            delegate: visitor,
            marker: PhantomData,
        })
    }

    fn deserialize_ignored_any<V>(self, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'arena>,
    {
        self.delegate.deserialize_ignored_any(ArenaDeserializer {
            arena: self.arena,
            delegate: visitor,
            marker: PhantomData,
        })
    }
}

impl<'arena, 'de, V> Visitor<'de> for ArenaDeserializer<'arena, 'de, V>
where
    V: Visitor<'arena>,
{
    type Value = <V as Visitor<'arena>>::Value;

    fn expecting(&self, formatter: &mut fmt::Formatter) -> fmt::Result {
        self.delegate.expecting(formatter)
    }

    fn visit_bool<E>(self, v: bool) -> Result<Self::Value, E>
    where
        E: serde::de::Error,
    {
        self.delegate.visit_bool(v)
    }

    fn visit_i8<E>(self, v: i8) -> Result<Self::Value, E>
    where
        E: serde::de::Error,
    {
        self.delegate.visit_i8(v)
    }

    fn visit_i16<E>(self, v: i16) -> Result<Self::Value, E>
    where
        E: serde::de::Error,
    {
        self.delegate.visit_i16(v)
    }

    fn visit_i32<E>(self, v: i32) -> Result<Self::Value, E>
    where
        E: serde::de::Error,
    {
        self.delegate.visit_i32(v)
    }

    fn visit_i64<E>(self, v: i64) -> Result<Self::Value, E>
    where
        E: serde::de::Error,
    {
        self.delegate.visit_i64(v)
    }

    fn visit_u8<E>(self, v: u8) -> Result<Self::Value, E>
    where
        E: serde::de::Error,
    {
        self.delegate.visit_u8(v)
    }

    fn visit_u16<E>(self, v: u16) -> Result<Self::Value, E>
    where
        E: serde::de::Error,
    {
        self.delegate.visit_u16(v)
    }

    fn visit_u32<E>(self, v: u32) -> Result<Self::Value, E>
    where
        E: serde::de::Error,
    {
        self.delegate.visit_u32(v)
    }

    fn visit_u64<E>(self, v: u64) -> Result<Self::Value, E>
    where
        E: serde::de::Error,
    {
        self.delegate.visit_u64(v)
    }

    fn visit_f32<E>(self, v: f32) -> Result<Self::Value, E>
    where
        E: serde::de::Error,
    {
        self.delegate.visit_f32(v)
    }

    fn visit_f64<E>(self, v: f64) -> Result<Self::Value, E>
    where
        E: serde::de::Error,
    {
        self.delegate.visit_f64(v)
    }

    fn visit_char<E>(self, v: char) -> Result<Self::Value, E>
    where
        E: serde::de::Error,
    {
        self.delegate.visit_char(v)
    }

    fn visit_str<E>(self, v: &str) -> Result<Self::Value, E>
    where
        E: serde::de::Error,
    {
        self.delegate.visit_str(v)
    }

    fn visit_string<E>(self, v: String) -> Result<Self::Value, E>
    where
        E: serde::de::Error,
    {
        self.delegate.visit_string(v)
    }

    fn visit_bytes<E>(self, v: &[u8]) -> Result<Self::Value, E>
    where
        E: serde::de::Error,
    {
        self.delegate.visit_bytes(v)
    }

    fn visit_byte_buf<E>(self, v: Vec<u8>) -> Result<Self::Value, E>
    where
        E: serde::de::Error,
    {
        self.delegate.visit_byte_buf(v)
    }

    fn visit_none<E>(self) -> Result<Self::Value, E>
    where
        E: serde::de::Error,
    {
        self.delegate.visit_none()
    }

    fn visit_some<D>(self, deserializer: D) -> Result<Self::Value, D::Error>
    where
        D: Deserializer<'de>,
    {
        self.delegate.visit_some(ArenaDeserializer {
            arena: self.arena,
            delegate: deserializer,
            marker: PhantomData,
        })
    }

    fn visit_unit<E>(self) -> Result<Self::Value, E>
    where
        E: serde::de::Error,
    {
        self.delegate.visit_unit()
    }

    fn visit_newtype_struct<D>(self, deserializer: D) -> Result<Self::Value, D::Error>
    where
        D: Deserializer<'de>,
    {
        self.delegate.visit_newtype_struct(ArenaDeserializer {
            arena: self.arena,
            delegate: deserializer,
            marker: PhantomData,
        })
    }

    fn visit_seq<A>(self, seq: A) -> Result<Self::Value, A::Error>
    where
        A: SeqAccess<'de>,
    {
        self.delegate.visit_seq(ArenaDeserializer {
            arena: self.arena,
            delegate: seq,
            marker: PhantomData,
        })
    }

    fn visit_map<M>(self, map: M) -> Result<Self::Value, M::Error>
    where
        M: MapAccess<'de>,
    {
        self.delegate.visit_map(ArenaDeserializer {
            arena: self.arena,
            delegate: map,
            marker: PhantomData,
        })
    }

    fn visit_enum<E>(self, data: E) -> Result<Self::Value, E::Error>
    where
        E: EnumAccess<'de>,
    {
        self.delegate.visit_enum(ArenaDeserializer {
            arena: self.arena,
            delegate: data,
            marker: PhantomData,
        })
    }
}

impl<'arena, 'de, S> SeqAccess<'arena> for ArenaDeserializer<'arena, 'de, S>
where
    S: SeqAccess<'de>,
{
    type Error = <S as SeqAccess<'de>>::Error;

    fn next_element_seed<K>(&mut self, seed: K) -> Result<Option<K::Value>, Self::Error>
    where
        K: DeserializeSeed<'arena>,
    {
        self.delegate.next_element_seed(ArenaDeserializer {
            arena: self.arena,
            delegate: seed,
            marker: PhantomData,
        })
    }
}

impl<'arena, 'de, M> MapAccess<'arena> for ArenaDeserializer<'arena, 'de, M>
where
    M: MapAccess<'de>,
{
    type Error = <M as MapAccess<'de>>::Error;

    fn next_key_seed<K>(&mut self, seed: K) -> Result<Option<K::Value>, Self::Error>
    where
        K: DeserializeSeed<'arena>,
    {
        self.delegate.next_key_seed(ArenaDeserializer {
            arena: self.arena,
            delegate: seed,
            marker: PhantomData,
        })
    }

    fn next_value_seed<V>(&mut self, seed: V) -> Result<V::Value, Self::Error>
    where
        V: DeserializeSeed<'arena>,
    {
        self.delegate.next_value_seed(ArenaDeserializer {
            arena: self.arena,
            delegate: seed,
            marker: PhantomData,
        })
    }
}

impl<'arena, 'de, E> EnumAccess<'arena> for ArenaDeserializer<'arena, 'de, E>
where
    E: EnumAccess<'de>,
{
    type Error = <E as EnumAccess<'de>>::Error;
    type Variant = ArenaDeserializer<'arena, 'de, <E as EnumAccess<'de>>::Variant>;

    fn variant_seed<V>(self, seed: V) -> Result<(V::Value, Self::Variant), Self::Error>
    where
        V: DeserializeSeed<'arena>,
    {
        let (value, variant) = self.delegate.variant_seed(ArenaDeserializer {
            arena: self.arena,
            delegate: seed,
            marker: PhantomData,
        })?;
        Ok((
            value,
            ArenaDeserializer {
                arena: self.arena,
                delegate: variant,
                marker: PhantomData,
            },
        ))
    }
}

impl<'arena, 'de, E> VariantAccess<'arena> for ArenaDeserializer<'arena, 'de, E>
where
    E: VariantAccess<'de>,
{
    type Error = <E as VariantAccess<'de>>::Error;

    fn unit_variant(self) -> Result<(), Self::Error> {
        self.delegate.unit_variant()
    }

    fn newtype_variant_seed<T>(self, seed: T) -> Result<T::Value, Self::Error>
    where
        T: DeserializeSeed<'arena>,
    {
        self.delegate.newtype_variant_seed(ArenaDeserializer {
            arena: self.arena,
            delegate: seed,
            marker: PhantomData,
        })
    }

    fn tuple_variant<V>(self, len: usize, visitor: V) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'arena>,
    {
        self.delegate.tuple_variant(
            len,
            ArenaDeserializer {
                arena: self.arena,
                delegate: visitor,
                marker: PhantomData,
            },
        )
    }

    fn struct_variant<V>(
        self,
        fields: &'static [&'static str],
        visitor: V,
    ) -> Result<V::Value, Self::Error>
    where
        V: Visitor<'arena>,
    {
        self.delegate.struct_variant(
            fields,
            ArenaDeserializer {
                arena: self.arena,
                delegate: visitor,
                marker: PhantomData,
            },
        )
    }
}

impl<'arena, 'de, S> DeserializeSeed<'de> for ArenaDeserializer<'arena, 'de, S>
where
    S: DeserializeSeed<'arena>,
{
    type Value = <S as DeserializeSeed<'arena>>::Value;

    fn deserialize<D>(self, deserializer: D) -> Result<Self::Value, D::Error>
    where
        D: Deserializer<'de>,
    {
        self.delegate.deserialize(ArenaDeserializer {
            arena: self.arena,
            delegate: deserializer,
            marker: PhantomData,
        })
    }
}

pub(crate) fn obtain_arena<'arena, D>(deserializer: &D) -> Result<&'arena Bump, D::Error>
where
    D: Deserializer<'arena>,
{
    // Not 100% bulletproof. The failure mode is if somebody were to make a
    // different crate with the same crate name and type name, and pass one of
    // their deserializers into here. This would only happen if somebody is
    // deliberately attacking this code, which realistically we do not expect.
    let deserializer_type_name = any::type_name::<D>();
    if !deserializer_type_name.starts_with("arena_deserializer::deserializer::ArenaDeserializer<") {
        return Err(serde::de::Error::custom(format!(
            "#[serde(deserialize_with = \"arena\")] used with non-ArenaDeserializer:  {}",
            deserializer_type_name,
        )));
    }

    // So D is ArenaDeserializer<'?a, '?b, ?C> for some unknown '?a, '?b, 'C.
    //
    // The Deserializer impl for ArenaDeserializer is:
    //
    //     impl<'arena, 'de, D> Deserializer<'arena> for ArenaDeserializer<'arena, 'de, D>
    //     where
    //         D: Deserializer<'de>;
    //
    // and we know ArenaDeserializer<'?a, '?b, ?C> impls Deserializer<'arena>,
    // therefore '?a == 'arena.
    //
    // The first field of ArenaDeserializer<'arena, '?b, ?C> is &'arena Bump so
    // we can grab that.
    let arena = unsafe { mem::transmute_copy::<D, &'arena Bump>(deserializer) };

    Ok(arena)
}
