/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use serde::de;
use serde::forward_to_deserialize_any;

use super::Deserializer;
use super::read::DeRead;
use super::reentrant::ReentrantGuard;
use crate::errors::*;
use crate::header::*;

pub struct MapAccess<'a, R> {
    de: &'a mut Deserializer<R>,
    remaining: usize,
}

impl<'a, 'de, R> MapAccess<'a, R>
where
    R: 'a + DeRead<'de>,
{
    /// Create a new `MapAccess`.
    ///
    /// `_guard` makes sure the caller is accounting for the recursion limit.
    pub fn new(de: &'a mut Deserializer<R>, nitems: usize, _guard: &ReentrantGuard) -> Self {
        MapAccess {
            de,
            remaining: nitems,
        }
    }
}

impl<'a, 'de, R> de::MapAccess<'de> for MapAccess<'a, R>
where
    R: 'a + DeRead<'de>,
{
    type Error = Error;

    fn next_key_seed<K>(&mut self, seed: K) -> Result<Option<K::Value>>
    where
        K: de::DeserializeSeed<'de>,
    {
        if self.remaining == 0 {
            Ok(None)
        } else {
            self.remaining -= 1;
            let key = seed.deserialize(MapKey { de: &mut *self.de })?;
            Ok(Some(key))
        }
    }

    fn next_value_seed<V>(&mut self, seed: V) -> Result<V::Value>
    where
        V: de::DeserializeSeed<'de>,
    {
        seed.deserialize(&mut *self.de)
    }
}

/// A deserializer that is specialized to deal with map keys. Specifically, map keys are always
/// strings.
struct MapKey<'a, R> {
    de: &'a mut Deserializer<R>,
}

impl<'a, 'de, R> de::Deserializer<'de> for MapKey<'a, R>
where
    R: DeRead<'de>,
{
    type Error = Error;

    #[inline]
    fn deserialize_any<V>(self, visitor: V) -> Result<V::Value>
    where
        V: de::Visitor<'de>,
    {
        match self.de.bunser.peek()? {
            // Both bytestrings and UTF-8 strings are treated as Unicode strings, since field
            // identifiers must be Unicode strings.
            BSER_BYTESTRING | BSER_UTF8STRING => self.de.visit_utf8string(visitor),
            other => Err(Error::DeInvalidStartByte {
                kind: "map key".into(),
                byte: other,
            }),
        }
    }

    #[inline]
    fn deserialize_option<V>(self, visitor: V) -> Result<V::Value>
    where
        V: de::Visitor<'de>,
    {
        // Map keys cannot be null.
        visitor.visit_some(self)
    }

    #[inline]
    fn deserialize_newtype_struct<V>(self, _name: &'static str, visitor: V) -> Result<V::Value>
    where
        V: de::Visitor<'de>,
    {
        visitor.visit_newtype_struct(self)
    }

    #[inline]
    fn deserialize_enum<V>(
        self,
        name: &'static str,
        variants: &'static [&'static str],
        visitor: V,
    ) -> Result<V::Value>
    where
        V: de::Visitor<'de>,
    {
        self.de.deserialize_enum(name, variants, visitor)
    }

    #[inline]
    fn deserialize_bytes<V>(self, visitor: V) -> Result<V::Value>
    where
        V: de::Visitor<'de>,
    {
        self.de.deserialize_bytes(visitor)
    }

    #[inline]
    fn deserialize_byte_buf<V>(self, visitor: V) -> Result<V::Value>
    where
        V: de::Visitor<'de>,
    {
        self.de.deserialize_bytes(visitor)
    }

    forward_to_deserialize_any! {
        bool i8 i16 i32 i64 u8 u16 u32 u64 f32 f64 char str string unit unit_struct
        seq tuple tuple_struct map struct identifier ignored_any
    }
}
