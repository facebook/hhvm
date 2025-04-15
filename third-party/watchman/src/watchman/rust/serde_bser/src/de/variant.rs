/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use serde::de;

use super::Deserializer;
use super::read::DeRead;
use super::reentrant::ReentrantGuard;
use crate::errors::*;

macro_rules! impl_enum_access {
    ($type:ident) => {
        impl<'a, 'de, R> de::EnumAccess<'de> for $type<'a, R>
        where
            R: 'a + DeRead<'de>,
        {
            type Error = Error;
            type Variant = Self;

            fn variant_seed<V>(self, seed: V) -> Result<(V::Value, Self)>
            where
                V: de::DeserializeSeed<'de>,
            {
                let val = seed.deserialize(&mut *self.de)?;
                Ok((val, self))
            }
        }
    };
}

/// Deserialize access for unit, struct and tuple variants.
pub struct VariantAccess<'a, R> {
    de: &'a mut Deserializer<R>,
}

impl<'a, 'de, R> VariantAccess<'a, R>
where
    R: 'a + DeRead<'de>,
{
    /// Create a new `VariantAccess`.
    ///
    /// `_guard` makes sure the caller is accounting for the recursion limit.
    pub fn new(de: &'a mut Deserializer<R>, _guard: &ReentrantGuard) -> Self {
        VariantAccess { de }
    }
}

impl_enum_access!(VariantAccess);

impl<'a, 'de, R> de::VariantAccess<'de> for VariantAccess<'a, R>
where
    R: 'a + DeRead<'de>,
{
    type Error = Error;

    fn unit_variant(self) -> Result<()> {
        de::Deserialize::deserialize(self.de)
    }

    fn newtype_variant_seed<T>(self, seed: T) -> Result<T::Value>
    where
        T: de::DeserializeSeed<'de>,
    {
        seed.deserialize(self.de)
    }

    fn tuple_variant<V>(self, _len: usize, visitor: V) -> Result<V::Value>
    where
        V: de::Visitor<'de>,
    {
        de::Deserializer::deserialize_any(self.de, visitor)
    }

    fn struct_variant<V>(self, _fields: &'static [&'static str], visitor: V) -> Result<V::Value>
    where
        V: de::Visitor<'de>,
    {
        de::Deserializer::deserialize_any(self.de, visitor)
    }
}

/// Deserialize access for plain unit variants.
pub struct UnitVariantAccess<'a, R> {
    de: &'a mut Deserializer<R>,
}

impl<'a, 'de, R> UnitVariantAccess<'a, R>
where
    R: 'a + DeRead<'de>,
{
    pub fn new(de: &'a mut Deserializer<R>) -> Self {
        UnitVariantAccess { de }
    }
}

impl_enum_access!(UnitVariantAccess);

impl<'a, 'de, R> de::VariantAccess<'de> for UnitVariantAccess<'a, R>
where
    R: 'a + DeRead<'de>,
{
    type Error = Error;

    fn unit_variant(self) -> Result<()> {
        Ok(())
    }

    fn newtype_variant_seed<T>(self, _seed: T) -> Result<T::Value>
    where
        T: de::DeserializeSeed<'de>,
    {
        Err(de::Error::invalid_type(
            de::Unexpected::UnitVariant,
            &"newtype variant",
        ))
    }

    fn tuple_variant<V>(self, _len: usize, _visitor: V) -> Result<V::Value>
    where
        V: de::Visitor<'de>,
    {
        Err(de::Error::invalid_type(
            de::Unexpected::UnitVariant,
            &"tuple variant",
        ))
    }

    fn struct_variant<V>(self, _fields: &'static [&'static str], _visitor: V) -> Result<V::Value>
    where
        V: de::Visitor<'de>,
    {
        Err(de::Error::invalid_type(
            de::Unexpected::UnitVariant,
            &"struct variant",
        ))
    }
}
