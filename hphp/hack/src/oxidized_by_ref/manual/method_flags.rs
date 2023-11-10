// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bitflags::bitflags;
use eq_modulo_pos::EqModuloPos;

// NB: Keep the values of these flags in sync with shallow_decl_defs.ml.

bitflags! {
    #[derive(EqModuloPos)]
    pub struct MethodFlags: u8 {
        const ABSTRACT            = 1 << 0;
        const FINAL               = 1 << 1;
        const OVERRIDE            = 1 << 2;
        const DYNAMICALLYCALLABLE = 1 << 3;
        const PHP_STD_LIB         = 1 << 4;
        const SUPPORT_DYNAMIC_TYPE = 1 << 5;
    }
}

impl MethodFlags {
    pub fn is_abstract(&self) -> bool {
        self.contains(Self::ABSTRACT)
    }
    pub fn is_final(&self) -> bool {
        self.contains(Self::FINAL)
    }
    pub fn is_dynamicallycallable(&self) -> bool {
        self.contains(Self::DYNAMICALLYCALLABLE)
    }
    pub fn is_override(&self) -> bool {
        self.contains(Self::OVERRIDE)
    }
    pub fn is_php_std_lib(&self) -> bool {
        self.contains(Self::PHP_STD_LIB)
    }
    pub fn supports_dynamic_type(&self) -> bool {
        self.contains(Self::SUPPORT_DYNAMIC_TYPE)
    }
}

impl ocamlrep::ToOcamlRep for MethodFlags {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&'a self, _alloc: &'a A) -> ocamlrep::Value<'a> {
        ocamlrep::Value::int(self.bits() as isize)
    }
}

impl ocamlrep::FromOcamlRep for MethodFlags {
    fn from_ocamlrep(value: ocamlrep::Value<'_>) -> Result<Self, ocamlrep::FromError> {
        let int_value = ocamlrep::from::expect_int(value)?;
        Ok(Self::from_bits_truncate(int_value.try_into()?))
    }
}

impl<'a> ocamlrep::FromOcamlRepIn<'a> for MethodFlags {
    fn from_ocamlrep_in(
        value: ocamlrep::Value<'_>,
        _alloc: &'a bumpalo::Bump,
    ) -> Result<Self, ocamlrep::FromError> {
        use ocamlrep::FromOcamlRep;
        Self::from_ocamlrep(value)
    }
}

impl no_pos_hash::NoPosHash for MethodFlags {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        std::hash::Hash::hash(self, state);
    }
}

impl serde::Serialize for MethodFlags {
    fn serialize<S: serde::Serializer>(&self, serializer: S) -> Result<S::Ok, S::Error> {
        serializer.serialize_u8(self.bits())
    }
}

impl<'de> serde::Deserialize<'de> for MethodFlags {
    fn deserialize<D: serde::Deserializer<'de>>(deserializer: D) -> Result<Self, D::Error> {
        struct Visitor;
        impl<'de> serde::de::Visitor<'de> for Visitor {
            type Value = MethodFlags;

            fn expecting(&self, formatter: &mut ::std::fmt::Formatter<'_>) -> ::std::fmt::Result {
                write!(formatter, "a u8 for MethodFlags")
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
