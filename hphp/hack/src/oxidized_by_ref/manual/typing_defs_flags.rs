// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::convert::TryInto;

use bitflags::bitflags;

// NB: Keep the values of these flags in sync with typing_defs_flags.ml.

bitflags! {
    pub struct FunTypeFlags: u16 {
        const RETURN_DISPOSABLE      = 1 << 0;
        const RETURNS_MUTABLE        = 1 << 1;
        const RETURNS_VOID_TO_RX     = 1 << 2;
        const IS_COROUTINE           = 1 << 3;
        const ASYNC                  = 1 << 4;
        const GENERATOR              = 1 << 5;

        // These flags apply to the self type on methods.
        const MUTABLE_FLAGS_OWNED    = 1 << 6;
        const MUTABLE_FLAGS_BORROWED = 1 << 7;
        const MUTABLE_FLAGS_MAYBE    = Self::MUTABLE_FLAGS_OWNED.bits | Self::MUTABLE_FLAGS_BORROWED.bits;
        const MUTABLE_FLAGS_MASK     = Self::MUTABLE_FLAGS_OWNED.bits | Self::MUTABLE_FLAGS_BORROWED.bits;

        const INSTANTIATED_TARGS     = 1 << 8;
        const IS_FUNCTION_POINTER    = 1 << 9;
    }
}

bitflags! {
    pub struct FunParamFlags: u8 {
        const ACCEPT_DISPOSABLE      = 1 << 0;
        const INOUT                  = 1 << 1;
        const HAS_DEFAULT            = 1 << 2;

        // These flags apply to the parameter type.
        const MUTABLE_FLAGS_OWNED    = 1 << 6;
        const MUTABLE_FLAGS_BORROWED = 1 << 7;
        const MUTABLE_FLAGS_MAYBE    = Self::MUTABLE_FLAGS_OWNED.bits | Self::MUTABLE_FLAGS_BORROWED.bits;
        const MUTABLE_FLAGS_MASK     = Self::MUTABLE_FLAGS_OWNED.bits | Self::MUTABLE_FLAGS_BORROWED.bits;
    }
}

impl ocamlrep::ToOcamlRep for FunTypeFlags {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&self, _alloc: &'a A) -> ocamlrep::OpaqueValue<'a> {
        ocamlrep::OpaqueValue::int(self.bits() as isize)
    }
}

impl ocamlrep::FromOcamlRep for FunTypeFlags {
    fn from_ocamlrep(value: ocamlrep::Value<'_>) -> Result<Self, ocamlrep::FromError> {
        let int_value = ocamlrep::from::expect_int(value)?;
        Ok(Self::from_bits_truncate(int_value.try_into()?))
    }
}

impl<'a> ocamlrep::FromOcamlRepIn<'a> for FunTypeFlags {
    fn from_ocamlrep_in(
        value: ocamlrep::Value<'_>,
        _alloc: &'a bumpalo::Bump,
    ) -> Result<Self, ocamlrep::FromError> {
        use ocamlrep::FromOcamlRep;
        Self::from_ocamlrep(value)
    }
}

impl serde::Serialize for FunTypeFlags {
    fn serialize<S: serde::Serializer>(&self, serializer: S) -> Result<S::Ok, S::Error> {
        serializer.serialize_u16(self.bits())
    }
}

impl<'de> serde::Deserialize<'de> for FunTypeFlags {
    fn deserialize<D: serde::Deserializer<'de>>(deserializer: D) -> Result<Self, D::Error> {
        struct Visitor;
        impl<'de> serde::de::Visitor<'de> for Visitor {
            type Value = FunTypeFlags;

            fn expecting(&self, formatter: &mut ::std::fmt::Formatter) -> ::std::fmt::Result {
                write!(formatter, "a u16 for FunTypeFlags")
            }
            fn visit_u16<E: serde::de::Error>(self, value: u16) -> Result<Self::Value, E> {
                Ok(Self::Value::from_bits_truncate(value))
            }
        }
        deserializer.deserialize_u16(Visitor)
    }
}

impl ocamlrep::ToOcamlRep for FunParamFlags {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&self, _alloc: &'a A) -> ocamlrep::OpaqueValue<'a> {
        ocamlrep::OpaqueValue::int(self.bits() as isize)
    }
}

impl ocamlrep::FromOcamlRep for FunParamFlags {
    fn from_ocamlrep(value: ocamlrep::Value<'_>) -> Result<Self, ocamlrep::FromError> {
        let int_value = ocamlrep::from::expect_int(value)?;
        Ok(Self::from_bits_truncate(int_value.try_into()?))
    }
}

impl<'a> ocamlrep::FromOcamlRepIn<'a> for FunParamFlags {
    fn from_ocamlrep_in(
        value: ocamlrep::Value<'_>,
        _alloc: &'a bumpalo::Bump,
    ) -> Result<Self, ocamlrep::FromError> {
        use ocamlrep::FromOcamlRep;
        Self::from_ocamlrep(value)
    }
}

impl serde::Serialize for FunParamFlags {
    fn serialize<S: serde::Serializer>(&self, serializer: S) -> Result<S::Ok, S::Error> {
        serializer.serialize_u8(self.bits())
    }
}

impl<'de> serde::Deserialize<'de> for FunParamFlags {
    fn deserialize<D: serde::Deserializer<'de>>(deserializer: D) -> Result<Self, D::Error> {
        struct Visitor;
        impl<'de> serde::de::Visitor<'de> for Visitor {
            type Value = FunParamFlags;

            fn expecting(&self, formatter: &mut ::std::fmt::Formatter) -> ::std::fmt::Result {
                write!(formatter, "a u8 for FunParamFlags")
            }
            fn visit_u8<E: serde::de::Error>(self, value: u8) -> Result<Self::Value, E> {
                Ok(Self::Value::from_bits_truncate(value))
            }
        }
        deserializer.deserialize_u8(Visitor)
    }
}
