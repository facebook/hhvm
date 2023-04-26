// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use eq_modulo_pos::EqModuloPos;
use hh24_types::ToplevelCanonSymbolHash;
use hh24_types::ToplevelSymbolHash;
use intern::string::BytesId;
use intern::string::IntoUtf8Bytes;
use intern::string::StringId;
use ocamlrep::FromOcamlRep;
use ocamlrep::FromOcamlRepIn;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

use crate::ToOxidized;

#[derive(Copy, Clone, PartialEq, Eq, Hash, Ord, PartialOrd)]
#[derive(Serialize, Deserialize)]
pub struct Symbol(pub StringId);
// nb: StringId implements Hash & Eq using the u32 id, and Ord
// using the underlying string after a fast check for equal ids.

impl Symbol {
    pub fn new<S: IntoUtf8Bytes>(s: S) -> Self {
        Self(intern::string::intern(s))
    }
}

impl Symbol {
    pub fn as_str(&self) -> &'static str {
        self.0.as_str()
    }

    pub fn as_bytes(&self) -> &'static [u8] {
        self.0.as_str().as_bytes()
    }
}

impl std::ops::Deref for Symbol {
    type Target = str;

    fn deref(&self) -> &str {
        self.as_str()
    }
}

impl std::convert::AsRef<str> for Symbol {
    fn as_ref(&self) -> &str {
        self.as_str()
    }
}

impl std::fmt::Debug for Symbol {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{:?}", self.as_str())
    }
}

impl std::fmt::Display for Symbol {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.as_str())
    }
}

impl<T: IntoUtf8Bytes> From<T> for Symbol {
    fn from(s: T) -> Self {
        Self::new(s)
    }
}

impl EqModuloPos for Symbol {
    fn eq_modulo_pos(&self, rhs: &Self) -> bool {
        self == rhs
    }
    fn eq_modulo_pos_and_reason(&self, rhs: &Self) -> bool {
        self == rhs
    }
}

impl<'a> ToOxidized<'a> for Symbol {
    type Output = &'a str;

    fn to_oxidized(&self, _arena: &'a bumpalo::Bump) -> &'a str {
        self.as_str()
    }
}

impl ToOcamlRep for Symbol {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&'a self, alloc: &'a A) -> ocamlrep::Value<'a> {
        alloc.add_copy(self.as_str())
    }
}

impl FromOcamlRep for Symbol {
    fn from_ocamlrep(value: ocamlrep::Value<'_>) -> Result<Self, ocamlrep::FromError> {
        Ok(Self::new(ocamlrep::str_from_ocamlrep(value)?))
    }
}

impl<'a> FromOcamlRepIn<'a> for Symbol {
    fn from_ocamlrep_in(
        value: ocamlrep::Value<'_>,
        _arena: &'a bumpalo::Bump,
    ) -> Result<Self, ocamlrep::FromError> {
        Ok(Self::new(ocamlrep::str_from_ocamlrep(value)?))
    }
}

impl arena_trait::TrivialDrop for Symbol {}

#[derive(Copy, Clone, PartialEq, Eq, Hash, Ord, PartialOrd)]
#[derive(Serialize, Deserialize)]
pub struct Bytes(pub BytesId);
// nb: BytesId implements Hash & Eq using the u32 id, and Ord
// using the underlying bytestring after a fast check for equal ids.

impl Bytes {
    pub const EMPTY: Bytes = Bytes(BytesId::EMPTY);

    pub fn new<S: AsRef<[u8]>>(s: S) -> Self {
        Self(intern::string::intern_bytes(s.as_ref()))
    }
}

impl Bytes {
    pub fn as_bytes(&self) -> &'static [u8] {
        self.0.as_bytes()
    }

    pub fn as_bstr(&self) -> &bstr::BStr {
        self.0.as_bytes().into()
    }
}

impl std::ops::Deref for Bytes {
    type Target = [u8];

    fn deref(&self) -> &[u8] {
        self.as_bytes()
    }
}

impl std::convert::AsRef<[u8]> for Bytes {
    fn as_ref(&self) -> &[u8] {
        self.as_bytes()
    }
}

impl std::fmt::Debug for Bytes {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{:?}", self.as_bstr())
    }
}

impl std::fmt::Display for Bytes {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.as_bstr())
    }
}

impl From<&[u8]> for Bytes {
    fn from(s: &[u8]) -> Self {
        Self::new(s)
    }
}

impl From<&bstr::BStr> for Bytes {
    fn from(s: &bstr::BStr) -> Self {
        Self::new(s)
    }
}

impl From<&str> for Bytes {
    fn from(s: &str) -> Self {
        Self::new(s)
    }
}

impl EqModuloPos for Bytes {
    fn eq_modulo_pos(&self, rhs: &Self) -> bool {
        self == rhs
    }
    fn eq_modulo_pos_and_reason(&self, rhs: &Self) -> bool {
        self == rhs
    }
}

impl<'a> ToOxidized<'a> for Bytes {
    type Output = &'a [u8];

    fn to_oxidized(&self, _arena: &'a bumpalo::Bump) -> &'a [u8] {
        self.0.as_bytes()
    }
}

impl ToOcamlRep for Bytes {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&'a self, alloc: &'a A) -> ocamlrep::Value<'a> {
        alloc.add_copy(self.as_bytes())
    }
}

impl FromOcamlRep for Bytes {
    fn from_ocamlrep(value: ocamlrep::Value<'_>) -> Result<Self, ocamlrep::FromError> {
        Ok(Self::new(ocamlrep::bytes_from_ocamlrep(value)?))
    }
}

impl<'a> FromOcamlRepIn<'a> for Bytes {
    fn from_ocamlrep_in(
        value: ocamlrep::Value<'_>,
        _arena: &'a bumpalo::Bump,
    ) -> Result<Self, ocamlrep::FromError> {
        Ok(Self::new(ocamlrep::bytes_from_ocamlrep(value)?))
    }
}

impl arena_trait::TrivialDrop for Bytes {}

macro_rules! common_impls {
    ($name:ident) => {
        impl $name {
            pub fn new<S: IntoUtf8Bytes>(s: S) -> Self {
                Self(Symbol::new(s))
            }

            pub fn as_str(&self) -> &'static str {
                self.0.as_str()
            }

            pub fn as_symbol(self) -> Symbol {
                self.0
            }
        }

        impl std::ops::Deref for $name {
            type Target = str;

            fn deref(&self) -> &str {
                self.as_str()
            }
        }

        impl std::convert::AsRef<str> for $name {
            fn as_ref(&self) -> &str {
                self.as_str()
            }
        }

        impl std::borrow::Borrow<str> for $name {
            fn borrow(&self) -> &str {
                self.as_str()
            }
        }

        impl std::borrow::Borrow<[u8]> for $name {
            fn borrow(&self) -> &[u8] {
                self.as_bytes()
            }
        }

        impl std::fmt::Debug for $name {
            fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
                write!(f, "{:?}", self.as_str())
            }
        }

        impl std::fmt::Display for $name {
            fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
                write!(f, "{}", self.as_str())
            }
        }

        impl<T: IntoUtf8Bytes> From<T> for $name {
            fn from(s: T) -> Self {
                Self::new(s)
            }
        }

        impl<'a> ToOxidized<'a> for $name {
            type Output = &'a str;

            fn to_oxidized(&self, _arena: &'a bumpalo::Bump) -> &'a str {
                self.as_str()
            }
        }

        impl ToOcamlRep for $name {
            fn to_ocamlrep<'a, A: ocamlrep::Allocator>(
                &'a self,
                alloc: &'a A,
            ) -> ocamlrep::Value<'a> {
                alloc.add_copy(self.as_str())
            }
        }

        impl FromOcamlRep for $name {
            fn from_ocamlrep(value: ocamlrep::Value<'_>) -> Result<Self, ocamlrep::FromError> {
                Ok(Self::new(ocamlrep::str_from_ocamlrep(value)?))
            }
        }

        impl<'a> FromOcamlRepIn<'a> for $name {
            fn from_ocamlrep_in(
                value: ocamlrep::Value<'_>,
                _arena: &'a bumpalo::Bump,
            ) -> Result<Self, ocamlrep::FromError> {
                Ok(Self::new(ocamlrep::str_from_ocamlrep(value)?))
            }
        }

        impl arena_trait::TrivialDrop for $name {}
    };
}

// The following newtype wrappers are all for name categories that are
// disjoint from each other.
// Toplevel names can have namespace qualifiers, unlike member names.
// Toplevel names are not case sensitive in HHVM
//
// Any one of these name wrappers could turn into an enum if necessary
// to avoid stringly typed mangled names during compilation.

/// A TypeName is the name of a class, interface, trait, type parameter,
/// type alias, newtype, or primitive type names like int, arraykey, etc.
#[derive(Eq, PartialEq, EqModuloPos, Clone, Copy, Hash, Ord, PartialOrd)]
#[derive(Serialize, Deserialize)]
pub struct TypeName(pub Symbol);
common_impls!(TypeName);

impl From<TypeName> for ToplevelSymbolHash {
    fn from(symbol: TypeName) -> Self {
        Self::from_type(symbol.as_str())
    }
}

impl From<TypeName> for ToplevelCanonSymbolHash {
    fn from(symbol: TypeName) -> Self {
        Self::from_type(symbol.as_str().to_owned())
    }
}

/// ModuleName is introduced by the experimental Modules feature and `internal`
/// visibility. ModuleNames are not bindable names and are not intended
/// to be interchangeable with any other kind of name.
#[derive(Eq, PartialEq, EqModuloPos, Clone, Copy, Hash, Ord, PartialOrd)]
#[derive(Serialize, Deserialize)]
pub struct ModuleName(pub Symbol);
common_impls!(ModuleName);

impl From<ModuleName> for ToplevelSymbolHash {
    fn from(symbol: ModuleName) -> Self {
        Self::from_module(symbol.as_str())
    }
}

/// Name of a top level constant.
#[derive(Eq, PartialEq, EqModuloPos, Clone, Copy, Hash, Ord, PartialOrd)]
#[derive(Serialize, Deserialize)]
pub struct ConstName(pub Symbol);
common_impls!(ConstName);

impl From<ConstName> for ToplevelSymbolHash {
    fn from(symbol: ConstName) -> Self {
        Self::from_const(symbol.as_str())
    }
}

/// Name of a top level function.
#[derive(Eq, PartialEq, EqModuloPos, Clone, Copy, Hash, Ord, PartialOrd)]
#[derive(Serialize, Deserialize)]
pub struct FunName(pub Symbol);
common_impls!(FunName);

impl From<FunName> for ToplevelSymbolHash {
    fn from(symbol: FunName) -> Self {
        Self::from_fun(symbol.as_str())
    }
}

impl From<FunName> for ToplevelCanonSymbolHash {
    fn from(symbol: FunName) -> Self {
        Self::from_fun(symbol.as_str().to_owned())
    }
}

/// ClassConstName is the name of a class const, which are disjoint from
/// global constants, type constants, and other class members.
#[derive(Eq, PartialEq, EqModuloPos, Clone, Copy, Hash, Ord, PartialOrd)]
#[derive(Serialize, Deserialize)]
pub struct ClassConstName(pub Symbol);
common_impls!(ClassConstName);

#[derive(Eq, PartialEq, EqModuloPos, Clone, Copy, Hash, Ord, PartialOrd)]
#[derive(Serialize, Deserialize)]
pub struct TypeConstName(pub Symbol);
common_impls!(TypeConstName);

#[derive(Eq, PartialEq, EqModuloPos, Clone, Copy, Hash, Ord, PartialOrd)]
#[derive(Serialize, Deserialize)]
pub struct MethodName(pub Symbol);
common_impls!(MethodName);

#[derive(Eq, PartialEq, EqModuloPos, Clone, Copy, Hash, Ord, PartialOrd)]
#[derive(Serialize, Deserialize)]
pub struct PropName(pub Symbol);
common_impls!(PropName);
