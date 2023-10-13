// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bitflags::bitflags;
use eq_modulo_pos::EqModuloPos;

use crate::xhp_attribute;
use crate::xhp_attribute::XhpAttribute;

// NB: Keep the values of these flags in sync with typing_defs_flags.ml.

bitflags! {
    #[derive(EqModuloPos)]
    pub struct FunTypeFlags: u16 {
        const RETURN_DISPOSABLE      = 1 << 0;
        const IS_COROUTINE           = 1 << 3;
        const ASYNC                  = 1 << 4;
        const GENERATOR              = 1 << 5;
        const INSTANTIATED_TARGS     = 1 << 8;
        const IS_FUNCTION_POINTER    = 1 << 9;
        const RETURNS_READONLY       = 1 << 10;
        const READONLY_THIS          = 1 << 11;
        const SUPPORT_DYNAMIC_TYPE   = 1 << 12;
        const IS_MEMOIZED            = 1 << 13;
        const VARIADIC               = 1 << 14;
    }
}

bitflags! {
    #[derive(EqModuloPos)]
    pub struct FunParamFlags: u16 {
        const ACCEPT_DISPOSABLE      = 1 << 0;
        const INOUT                  = 1 << 1;
        const HAS_DEFAULT            = 1 << 2;
        const IFC_EXTERNAL           = 1 << 3;
        const IFC_CAN_CALL           = 1 << 4;
        const READONLY       = 1 << 8;
    }
}

bitflags! {
    #[derive(EqModuloPos)]
    pub struct ClassEltFlags: u16 {
        const ABSTRACT                 = 1 << 0;
        const FINAL                    = 1 << 1;
        const SUPERFLUOUS_OVERRIDE     = 1 << 2;
        //Whether the __Override attribute is erroneous, i.e. there is
        // nothing in parents to override. This is set during decling
        // (because that's the easiest place to spot this error) so
        // that an error can be emitted later during typing.
        const LSB                      = 1 << 3;
        const SYNTHESIZED              = 1 << 4;
        const CONST                    = 1 << 5;
        const LATEINIT                 = 1 << 6;
        const DYNAMICALLYCALLABLE      = 1 << 7;
        const SUPPORT_DYNAMIC_TYPE     = 1 << 8;
        const XA_HAS_DEFAULT           = 1 << 9;
        const XA_TAG_REQUIRED          = 1 << 10;
        const XA_TAG_LATEINIT          = 1 << 11;
        const READONLY_PROP            = 1 << 12;
        const NEEDS_INIT               = 1 << 13;
        const SAFE_GLOBAL_VARIABLE     = 1 << 14;

        const XA_FLAGS_MASK = Self::XA_HAS_DEFAULT.bits | Self::XA_TAG_REQUIRED.bits | Self::XA_TAG_LATEINIT.bits;
    }
}

#[derive(Copy, Clone, Debug)]
pub struct ClassEltFlagsArgs {
    pub xhp_attr: Option<XhpAttribute>,
    pub is_abstract: bool,
    pub is_final: bool,
    pub is_superfluous_override: bool,
    pub is_lsb: bool,
    pub is_synthesized: bool,
    pub is_const: bool,
    pub is_lateinit: bool,
    pub is_dynamicallycallable: bool,
    pub is_readonly_prop: bool,
    pub supports_dynamic_type: bool,
    pub needs_init: bool,
    pub safe_global_variable: bool,
}

impl From<xhp_attribute::Tag> for ClassEltFlags {
    fn from(tag: xhp_attribute::Tag) -> Self {
        use xhp_attribute::Tag;
        match tag {
            Tag::Required => Self::XA_TAG_REQUIRED,
            Tag::LateInit => Self::XA_TAG_LATEINIT,
        }
    }
}

impl From<Option<XhpAttribute>> for ClassEltFlags {
    fn from(xhp_attr: Option<XhpAttribute>) -> Self {
        let mut flags = Self::empty();
        if let Some(XhpAttribute { tag, has_default }) = xhp_attr {
            if let Some(tag) = tag {
                flags.insert(Self::from(tag));
            } else {
                flags.insert(Self::XA_TAG_REQUIRED | Self::XA_TAG_LATEINIT);
            }
            if has_default {
                flags.insert(Self::XA_HAS_DEFAULT);
            }
        }
        flags
    }
}

impl ClassEltFlags {
    pub fn new(args: ClassEltFlagsArgs) -> Self {
        let ClassEltFlagsArgs {
            xhp_attr,
            is_abstract,
            is_final,
            is_superfluous_override,
            is_lsb,
            is_synthesized,
            is_const,
            is_lateinit,
            is_dynamicallycallable,
            is_readonly_prop,
            supports_dynamic_type,
            needs_init,
            safe_global_variable,
        } = args;
        let mut flags = Self::empty();
        flags.set(Self::ABSTRACT, is_abstract);
        flags.set(Self::FINAL, is_final);
        flags.set(Self::SUPERFLUOUS_OVERRIDE, is_superfluous_override);
        flags.set(Self::LSB, is_lsb);
        flags.set(Self::SYNTHESIZED, is_synthesized);
        flags.set(Self::CONST, is_const);
        flags.set(Self::LATEINIT, is_lateinit);
        flags.set(Self::DYNAMICALLYCALLABLE, is_dynamicallycallable);
        flags.set_xhp_attr(xhp_attr);
        flags.set(Self::READONLY_PROP, is_readonly_prop);
        flags.set(Self::SUPPORT_DYNAMIC_TYPE, supports_dynamic_type);
        flags.set(Self::NEEDS_INIT, needs_init);
        flags.set(Self::SAFE_GLOBAL_VARIABLE, safe_global_variable);
        flags
    }

    pub fn get_xhp_attr(&self) -> Option<XhpAttribute> {
        use xhp_attribute::Tag;
        if self.intersection(Self::XA_FLAGS_MASK).is_empty() {
            return None;
        }
        let has_default = self.contains(Self::XA_HAS_DEFAULT);
        let tag = match (
            self.contains(Self::XA_TAG_REQUIRED),
            self.contains(Self::XA_TAG_LATEINIT),
        ) {
            (true, false) => Some(Tag::Required),
            (false, true) => Some(Tag::LateInit),
            // If both `XaTagRequired` and `XaTagLateinit` bits are set then
            // that's code for tag = None.
            (true, true) | (false, false) => None,
        };
        Some(XhpAttribute { tag, has_default })
    }

    fn set_xhp_attr(&mut self, xa: Option<XhpAttribute>) {
        self.remove(Self::XA_FLAGS_MASK);
        self.insert(Self::from(xa));
    }
}

// This alias is only necessary because of how typing_defs.rs is
// generated (T111144107). TODO(SF, 2022-02-01): Change
// `typing_defs_flags.ml` so that this is no longer necessary.
pub mod class_elt {
    use super::ClassEltFlags;
    pub type ClassElt = ClassEltFlags;
}

pub mod fun {
    use super::FunTypeFlags;
    pub type Fun = FunTypeFlags;
}

impl ocamlrep::ToOcamlRep for ClassEltFlags {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&'a self, _alloc: &'a A) -> ocamlrep::Value<'a> {
        ocamlrep::Value::int(self.bits() as isize)
    }
}

impl ocamlrep::FromOcamlRep for ClassEltFlags {
    fn from_ocamlrep(value: ocamlrep::Value<'_>) -> Result<Self, ocamlrep::FromError> {
        let int_value = ocamlrep::from::expect_int(value)?;
        Ok(Self::from_bits_truncate(int_value.try_into()?))
    }
}

impl<'a> ocamlrep::FromOcamlRepIn<'a> for ClassEltFlags {
    fn from_ocamlrep_in(
        value: ocamlrep::Value<'_>,
        _alloc: &'a bumpalo::Bump,
    ) -> Result<Self, ocamlrep::FromError> {
        use ocamlrep::FromOcamlRep;
        Self::from_ocamlrep(value)
    }
}

impl no_pos_hash::NoPosHash for ClassEltFlags {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        std::hash::Hash::hash(self, state);
    }
}

impl serde::Serialize for ClassEltFlags {
    fn serialize<S: serde::Serializer>(&self, serializer: S) -> Result<S::Ok, S::Error> {
        serializer.serialize_u16(self.bits())
    }
}

impl<'de> serde::Deserialize<'de> for ClassEltFlags {
    fn deserialize<D: serde::Deserializer<'de>>(deserializer: D) -> Result<Self, D::Error> {
        struct Visitor;
        impl<'de> serde::de::Visitor<'de> for Visitor {
            type Value = ClassEltFlags;

            fn expecting(&self, formatter: &mut ::std::fmt::Formatter<'_>) -> ::std::fmt::Result {
                write!(formatter, "a u16 for ClassEltFlags")
            }
            fn visit_u16<E: serde::de::Error>(self, value: u16) -> Result<Self::Value, E> {
                Ok(Self::Value::from_bits_truncate(value))
            }

            fn visit_u64<E: serde::de::Error>(self, value: u64) -> Result<Self::Value, E> {
                Ok(Self::Value::from_bits_truncate(
                    u16::try_from(value).expect("expect an u16, but got u64"),
                ))
            }
        }
        deserializer.deserialize_u16(Visitor)
    }
}

impl ocamlrep::ToOcamlRep for FunTypeFlags {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&'a self, _alloc: &'a A) -> ocamlrep::Value<'a> {
        ocamlrep::Value::int(self.bits() as isize)
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

impl no_pos_hash::NoPosHash for FunTypeFlags {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        std::hash::Hash::hash(self, state);
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

            fn expecting(&self, formatter: &mut ::std::fmt::Formatter<'_>) -> ::std::fmt::Result {
                write!(formatter, "a u16 for FunTypeFlags")
            }
            fn visit_u16<E: serde::de::Error>(self, value: u16) -> Result<Self::Value, E> {
                Ok(Self::Value::from_bits_truncate(value))
            }

            fn visit_u64<E: serde::de::Error>(self, value: u64) -> Result<Self::Value, E> {
                Ok(Self::Value::from_bits_truncate(
                    u16::try_from(value).expect("expect an u16, but got u64"),
                ))
            }
        }
        deserializer.deserialize_u16(Visitor)
    }
}

impl ocamlrep::ToOcamlRep for FunParamFlags {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&'a self, _alloc: &'a A) -> ocamlrep::Value<'a> {
        ocamlrep::Value::int(self.bits() as isize)
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

impl no_pos_hash::NoPosHash for FunParamFlags {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        std::hash::Hash::hash(self, state);
    }
}

impl serde::Serialize for FunParamFlags {
    fn serialize<S: serde::Serializer>(&self, serializer: S) -> Result<S::Ok, S::Error> {
        serializer.serialize_u16(self.bits())
    }
}

impl<'de> serde::Deserialize<'de> for FunParamFlags {
    fn deserialize<D: serde::Deserializer<'de>>(deserializer: D) -> Result<Self, D::Error> {
        struct Visitor;
        impl<'de> serde::de::Visitor<'de> for Visitor {
            type Value = FunParamFlags;

            fn expecting(&self, formatter: &mut ::std::fmt::Formatter<'_>) -> ::std::fmt::Result {
                write!(formatter, "a u16 for FunParamFlags")
            }
            fn visit_u16<E: serde::de::Error>(self, value: u16) -> Result<Self::Value, E> {
                Ok(Self::Value::from_bits_truncate(value))
            }

            fn visit_u64<E: serde::de::Error>(self, value: u64) -> Result<Self::Value, E> {
                Ok(Self::Value::from_bits_truncate(
                    u16::try_from(value).expect("expect an u16, but got u64"),
                ))
            }
        }
        deserializer.deserialize_u16(Visitor)
    }
}
