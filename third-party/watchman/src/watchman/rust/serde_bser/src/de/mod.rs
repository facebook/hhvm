/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

mod bunser;
mod map;
mod read;
mod reentrant;
mod seq;
mod template;
#[cfg(test)]
mod test;
mod variant;

use std::io;
use std::str;

use serde::de;
use serde::forward_to_deserialize_any;

pub use self::bunser::Bunser;
pub use self::bunser::PduInfo;
pub use self::read::DeRead;
pub use self::read::Reference;
pub use self::read::SliceRead;
use self::reentrant::ReentrantLimit;
use crate::errors::*;
use crate::header::*;

pub struct Deserializer<R> {
    bunser: Bunser<R>,
    pdu_info: PduInfo,
    remaining_depth: ReentrantLimit,
}

macro_rules! make_visit_num {
    ($fn:ident, $next:ident) => {
        #[inline]
        fn $fn<V>(&mut self, visitor: V) -> Result<V::Value>
        where
            V: de::Visitor<'de>,
        {
            visitor.$fn(self.bunser.$next()?)
        }
    };
}

fn from_trait<'de, R, T>(read: R) -> Result<T>
where
    R: DeRead<'de>,
    T: de::Deserialize<'de>,
{
    let mut d = Deserializer::new(read)?;
    let value = de::Deserialize::deserialize(&mut d)?;

    // Make sure we saw the expected length.
    d.end()?;
    Ok(value)
}

pub fn from_slice<'de, T>(slice: &'de [u8]) -> Result<T>
where
    T: de::Deserialize<'de>,
{
    from_trait(SliceRead::new(slice))
}

pub fn from_reader<R, T>(rdr: R) -> Result<T>
where
    R: io::Read,
    T: de::DeserializeOwned,
{
    from_trait(read::IoRead::new(rdr))
}

impl<'de, R> Deserializer<R>
where
    R: DeRead<'de>,
{
    pub fn new(read: R) -> Result<Self> {
        let mut bunser = Bunser::new(read);
        let pdu_info = bunser.read_pdu()?;
        Ok(Deserializer {
            bunser,
            pdu_info,
            remaining_depth: ReentrantLimit::new(128),
        })
    }

    /// This method must be called after a value has been fully deserialized.
    pub fn end(&self) -> Result<()> {
        self.bunser.end(&self.pdu_info)
    }

    #[inline]
    pub fn capabilities(&self) -> u32 {
        self.pdu_info.bser_capabilities
    }

    fn parse_value<V>(&mut self, visitor: V) -> Result<V::Value>
    where
        V: de::Visitor<'de>,
    {
        match self.bunser.peek()? {
            BSER_ARRAY => {
                let guard = self.remaining_depth.acquire("array")?;
                self.bunser.discard();
                let nitems = self.bunser.check_next_int()?;

                visitor.visit_seq(seq::SeqAccess::new(self, nitems as usize, &guard))
            }
            BSER_OBJECT => {
                let guard = self.remaining_depth.acquire("object")?;
                self.bunser.discard();
                let nitems = self.bunser.check_next_int()?;

                visitor.visit_map(map::MapAccess::new(self, nitems as usize, &guard))
            }
            BSER_TRUE => self.visit_bool(visitor, true),
            BSER_FALSE => self.visit_bool(visitor, false),
            BSER_NULL => self.visit_unit(visitor),
            BSER_BYTESTRING => self.visit_bytestring(visitor),
            BSER_UTF8STRING => self.visit_utf8string(visitor),
            BSER_TEMPLATE => {
                let guard = self.remaining_depth.acquire("template")?;
                self.bunser.discard();

                // TODO: handle possible IO interruption better here -- will
                // probably need some intermediate states.
                let keys = self.template_keys()?;
                let nitems = self.bunser.check_next_int()?;
                let template = template::Template::new(self, keys, nitems as usize, &guard);
                visitor.visit_seq(template)
            }
            BSER_REAL => self.visit_f64(visitor),
            BSER_INT8 => self.visit_i8(visitor),
            BSER_INT16 => self.visit_i16(visitor),
            BSER_INT32 => self.visit_i32(visitor),
            BSER_INT64 => self.visit_i64(visitor),
            ch => Err(Error::DeInvalidStartByte {
                kind: "next item".into(),
                byte: ch,
            }),
        }
    }

    make_visit_num!(visit_i8, next_i8);
    make_visit_num!(visit_i16, next_i16);
    make_visit_num!(visit_i32, next_i32);
    make_visit_num!(visit_i64, next_i64);
    make_visit_num!(visit_f64, next_f64);

    fn template_keys(&mut self) -> Result<Vec<template::Key<'de>>> {
        // The list of keys is actually an array, so just use the deserializer
        // to process it.
        de::Deserialize::deserialize(self)
    }

    fn visit_bytestring<V>(&mut self, visitor: V) -> Result<V::Value>
    where
        V: de::Visitor<'de>,
    {
        self.bunser.discard();
        let len = self.bunser.check_next_int()?;
        match self.bunser.read_bytes(len)? {
            Reference::Borrowed(s) => visitor.visit_borrowed_bytes(s),
            Reference::Copied(s) => visitor.visit_bytes(s),
        }
    }

    fn visit_utf8string<V>(&mut self, visitor: V) -> Result<V::Value>
    where
        V: de::Visitor<'de>,
    {
        self.bunser.discard();
        let len = self.bunser.check_next_int()?;
        match self
            .bunser
            .read_bytes(len)?
            .map_result(str::from_utf8)
            .map_err(Error::de_reader_error)?
        {
            Reference::Borrowed(s) => visitor.visit_borrowed_str(s),
            Reference::Copied(s) => visitor.visit_str(s),
        }
    }

    #[inline]
    fn visit_bool<V>(&mut self, visitor: V, value: bool) -> Result<V::Value>
    where
        V: de::Visitor<'de>,
    {
        self.bunser.discard();
        visitor.visit_bool(value)
    }

    #[inline]
    fn visit_unit<V>(&mut self, visitor: V) -> Result<V::Value>
    where
        V: de::Visitor<'de>,
    {
        self.bunser.discard();
        visitor.visit_unit()
    }
}

impl<'de, 'a, R> de::Deserializer<'de> for &'a mut Deserializer<R>
where
    R: DeRead<'de>,
{
    type Error = Error;

    #[inline]
    fn deserialize_any<V>(self, visitor: V) -> Result<V::Value>
    where
        V: de::Visitor<'de>,
    {
        self.parse_value(visitor)
    }

    /// Parse a `null` as a None, and anything else as a `Some(...)`.
    #[inline]
    fn deserialize_option<V>(self, visitor: V) -> Result<V::Value>
    where
        V: de::Visitor<'de>,
    {
        match self.bunser.peek()? {
            BSER_NULL => {
                self.bunser.discard();
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

    /// Parse an enum as an object like {key: value}, or a unit variant as just
    /// a value.
    #[inline]
    fn deserialize_enum<V>(
        self,
        name: &'static str,
        _variants: &'static [&'static str],
        visitor: V,
    ) -> Result<V::Value>
    where
        V: de::Visitor<'de>,
    {
        match self.bunser.peek()? {
            BSER_BYTESTRING | BSER_UTF8STRING => {
                visitor.visit_enum(variant::UnitVariantAccess::new(self))
            }
            BSER_OBJECT => {
                let guard = self
                    .remaining_depth
                    .acquire(format!("object-like enum '{}'", name))?;
                self.bunser.discard();
                // For enum variants the object must have exactly one entry
                // (named the variant, but serde will perform that check).
                let nitems = self.bunser.check_next_int()?;
                if nitems != 1 {
                    return Err(de::Error::invalid_value(
                        de::Unexpected::Signed(nitems),
                        &"integer `1`",
                    ));
                }
                visitor.visit_enum(variant::VariantAccess::new(self, &guard))
            }
            ch => Err(Error::DeInvalidStartByte {
                kind: format!("enum '{}'", name),
                byte: ch,
            }),
        }
    }

    forward_to_deserialize_any! {
        bool i8 i16 i32 i64 u8 u16 u32 u64 f32 f64 char str string bytes
        byte_buf unit unit_struct seq tuple tuple_struct map struct identifier
        ignored_any
    }
}
