/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::borrow::Cow;
use std::rc::Rc;

use serde::Deserialize;
use serde::de;
use serde::forward_to_deserialize_any;

use super::Deserializer;
use super::read::DeRead;
use super::reentrant::ReentrantGuard;
use crate::errors::*;
use crate::header::*;

// This is ugly because #[serde(borrow)] can't be used with collections directly
// at the moment. See
// https://github.com/serde-rs/serde/issues/914#issuecomment-298801226 for more.
// Note that keys are always ASCII, so treating them as str is fine, even if they're
// serialized as BSER_BYTESTRING instances. (These keys are used as struct field
// identifiers, which are Unicode strings so can't be directly matched up with
// bytestrings.)
#[derive(Clone, Debug, Deserialize)]
pub struct Key<'a>(#[serde(borrow)] Cow<'a, str>);

/// A BSER template is logically an array of objects, all with the same or
/// similar keys.
///
/// A template is serialized as
/// `<BSER_TEMPLATE><BSER_ARRAY of keys><number of objects><object 1 values><object 2 values>...`
///
/// and gets deserialized as
///
/// ```text
/// [
///    {key1: obj1_value1, key2: obj1_value2, ...},
///    {key2: obj2_value1, key2: obj2_value2, ...},
///    ...
/// ]
/// ```
///
/// The special value BSER_SKIP is used if a particular object doesn't have a
/// key.
pub struct Template<'a, 'de, R> {
    de: &'a mut Deserializer<R>,
    keys: Rc<Vec<Key<'de>>>,
    remaining: usize,
}

impl<'a, 'de, R: 'a> Template<'a, 'de, R> {
    /// Create a new `Template`.
    ///
    /// `_guard` makes sure the caller is accounting for the recursion limit.
    pub fn new(
        de: &'a mut Deserializer<R>,
        keys: Vec<Key<'de>>,
        nitems: usize,
        _guard: &ReentrantGuard,
    ) -> Self {
        Template {
            de,
            keys: Rc::new(keys),
            remaining: nitems,
        }
    }
}

impl<'a, 'de, R> de::SeqAccess<'de> for Template<'a, 'de, R>
where
    R: 'a + DeRead<'de>,
{
    type Error = Error;

    fn next_element_seed<T>(&mut self, seed: T) -> Result<Option<T::Value>>
    where
        T: de::DeserializeSeed<'de>,
    {
        if self.remaining == 0 {
            Ok(None)
        } else {
            self.remaining -= 1;
            let obj_de = ObjectDeserializer {
                de: &mut *self.de,
                keys: self.keys.clone(),
            };
            let value = seed.deserialize(obj_de)?;
            Ok(Some(value))
        }
    }
}

struct ObjectDeserializer<'a, 'de, R> {
    de: &'a mut Deserializer<R>,
    keys: Rc<Vec<Key<'de>>>,
}

impl<'a, 'de, R> de::Deserializer<'de> for ObjectDeserializer<'a, 'de, R>
where
    R: 'a + DeRead<'de>,
{
    type Error = Error;

    #[inline]
    fn deserialize_any<V>(self, visitor: V) -> Result<V::Value>
    where
        V: de::Visitor<'de>,
    {
        visitor.visit_map(TemplateObject::new(&mut *self.de, self.keys))
    }

    #[inline]
    fn deserialize_newtype_struct<V>(self, _name: &'static str, visitor: V) -> Result<V::Value>
    where
        V: de::Visitor<'de>,
    {
        // This is e.g. E(T). Ignore the E.
        visitor.visit_newtype_struct(self)
    }

    fn deserialize_enum<V>(
        self,
        _name: &'static str,
        _variants: &'static [&'static str],
        visitor: V,
    ) -> Result<V::Value>
    where
        V: de::Visitor<'de>,
    {
        visitor.visit_map(TemplateObject::new(&mut *self.de, self.keys))
    }

    forward_to_deserialize_any! {
        bool i8 i16 i32 i64 u8 u16 u32 u64 f32 f64 char str string bytes
        byte_buf unit unit_struct seq tuple tuple_struct map struct identifier
        ignored_any option
    }
}

struct TemplateObject<'a, 'de, R> {
    de: &'a mut Deserializer<R>,
    keys: Rc<Vec<Key<'de>>>,
    cur: usize,
}

impl<'a, 'de, R> TemplateObject<'a, 'de, R>
where
    R: 'a + DeRead<'de>,
{
    fn new(de: &'a mut Deserializer<R>, keys: Rc<Vec<Key<'de>>>) -> Self {
        TemplateObject { de, keys, cur: 0 }
    }
}

impl<'a, 'de, R> de::MapAccess<'de> for TemplateObject<'a, 'de, R>
where
    R: 'a + DeRead<'de>,
{
    type Error = Error;

    fn next_key_seed<K>(&mut self, seed: K) -> Result<Option<K::Value>>
    where
        K: de::DeserializeSeed<'de>,
    {
        if self.cur == self.keys.len() {
            Ok(None)
        } else {
            let cur = self.cur;
            self.cur += 1;
            let obj_de = KeyDeserializer {
                key: &self.keys[cur],
            };
            let value = seed.deserialize(obj_de)?;
            Ok(Some(value))
        }
    }

    fn next_value_seed<V>(&mut self, seed: V) -> Result<V::Value>
    where
        V: de::DeserializeSeed<'de>,
    {
        seed.deserialize(ValueDeserializer { de: &mut *self.de })
    }
}

struct KeyDeserializer<'a, 'de> {
    key: &'a Key<'de>,
}

impl<'a, 'de: 'a> de::Deserializer<'de> for KeyDeserializer<'a, 'de> {
    type Error = Error;

    #[inline]
    fn deserialize_any<V>(self, visitor: V) -> Result<V::Value>
    where
        V: de::Visitor<'de>,
    {
        match self.key.0 {
            Cow::Borrowed(s) => visitor.visit_borrowed_str(s),
            Cow::Owned(ref s) => visitor.visit_str(s),
        }
    }

    forward_to_deserialize_any! {
        bool i8 i16 i32 i64 u8 u16 u32 u64 f32 f64 char str string bytes
        byte_buf unit unit_struct seq tuple tuple_struct map struct identifier
        ignored_any option enum newtype_struct
    }
}

struct ValueDeserializer<'a, R> {
    de: &'a mut Deserializer<R>,
}

impl<'a, 'de, R> de::Deserializer<'de> for ValueDeserializer<'a, R>
where
    R: 'a + DeRead<'de>,
{
    type Error = Error;

    #[inline]
    fn deserialize_any<V>(self, visitor: V) -> Result<V::Value>
    where
        V: de::Visitor<'de>,
    {
        // The only new thing here compared to the main Deserializer is that it
        // is possible to skip this value with BSER_SKIP.
        match self.de.bunser.peek()? {
            BSER_SKIP => {
                self.de.bunser.discard();
                visitor.visit_none()
            }
            _ => self.de.deserialize_any(visitor),
        }
    }

    /// Parse a BSER_SKIP or a null as a None, and anything else as a
    /// `Some(...)`.
    #[inline]
    fn deserialize_option<V>(self, visitor: V) -> Result<V::Value>
    where
        V: de::Visitor<'de>,
    {
        match self.de.bunser.peek()? {
            BSER_SKIP | BSER_NULL => {
                self.de.bunser.discard();
                visitor.visit_none()
            }
            _ => visitor.visit_some(self),
        }
    }

    #[inline]
    fn deserialize_newtype_struct<V>(self, _name: &str, visitor: V) -> Result<V::Value>
    where
        V: de::Visitor<'de>,
    {
        // This is e.g. E(T). Ignore the E.
        visitor.visit_newtype_struct(self)
    }

    // TODO: do we also need to do enum here?

    forward_to_deserialize_any! {
        bool i8 i16 i32 i64 u8 u16 u32 u64 f32 f64 char str string bytes
        byte_buf unit unit_struct seq tuple tuple_struct map struct identifier
        ignored_any enum
    }
}
