// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::convert::{TryFrom, TryInto};

use bitflags::bitflags;
use eq_modulo_pos::EqModuloPos;

// NB: Keep the values of these flags in sync with shallow_decl_defs.ml.

bitflags! {
    #[derive(EqModuloPos)]
    pub struct PropFlags: u8 {
        const ABSTRACT    = 1 << 0;
        const CONST       = 1 << 1;
        const LATEINIT    = 1 << 2;
        const LSB         = 1 << 3;
        const NEEDS_INIT  = 1 << 4;
        const PHP_STD_LIB = 1 << 5;
        const READONLY    = 1 << 6;
    }
}

impl PropFlags {
    pub fn is_abstract(&self) -> bool {
        self.contains(Self::ABSTRACT)
    }
}

impl ocamlrep::ToOcamlRep for PropFlags {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&self, _alloc: &'a A) -> ocamlrep::OpaqueValue<'a> {
        ocamlrep::OpaqueValue::int(self.bits() as isize)
    }
}

impl ocamlrep::FromOcamlRep for PropFlags {
    fn from_ocamlrep(value: ocamlrep::Value<'_>) -> Result<Self, ocamlrep::FromError> {
        let int_value = ocamlrep::from::expect_int(value)?;
        Ok(Self::from_bits_truncate(int_value.try_into()?))
    }
}

impl<'a> ocamlrep::FromOcamlRepIn<'a> for PropFlags {
    fn from_ocamlrep_in(
        value: ocamlrep::Value<'_>,
        _alloc: &'a bumpalo::Bump,
    ) -> Result<Self, ocamlrep::FromError> {
        use ocamlrep::FromOcamlRep;
        Self::from_ocamlrep(value)
    }
}

impl no_pos_hash::NoPosHash for PropFlags {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        std::hash::Hash::hash(self, state);
    }
}

impl serde::Serialize for PropFlags {
    fn serialize<S: serde::Serializer>(&self, serializer: S) -> Result<S::Ok, S::Error> {
        serializer.serialize_u8(self.bits())
    }
}

impl<'de> serde::Deserialize<'de> for PropFlags {
    fn deserialize<D: serde::Deserializer<'de>>(deserializer: D) -> Result<Self, D::Error> {
        struct Visitor;
        impl<'de> serde::de::Visitor<'de> for Visitor {
            type Value = PropFlags;

            fn expecting(&self, formatter: &mut ::std::fmt::Formatter) -> ::std::fmt::Result {
                write!(formatter, "a u8 for PropFlags")
            }

            fn visit_u8<E: serde::de::Error>(self, value: u8) -> Result<Self::Value, E> {
                Ok(Self::Value::from_bits_truncate(value))
            }

            fn visit_u64<E: serde::de::Error>(self, value: u64) -> Result<Self::Value, E> {
                Ok(Self::Value::from_bits_truncate(
                    u8::try_from(value).expect("expect an u8, but got u64"),
                ))
            }
        }
        deserializer.deserialize_u8(Visitor)
    }
}
