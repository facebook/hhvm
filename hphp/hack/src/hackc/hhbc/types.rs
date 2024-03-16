// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use ffi::Vector;
use hhvm_types_ffi::ffi::TypeConstraintFlags;
use intern::string::StringId;
use naming_special_names_rust as naming_special_names;
use serde::Serialize;

use crate::TypedValue;

/// A TypeInfo represents a type written by the user. It consists of the type
/// written by the user (including generics) and an enforced constraint.
#[derive(Clone, Debug, Eq, PartialEq, Serialize)]
#[repr(C)]
pub struct TypeInfo {
    /// The type from source text that the user wrote including generics and special
    /// chars (like '?'). If None then this is directly computable from the constraint.
    pub user_type: Maybe<StringId>,

    /// The underlying type this TypeInfo is constrained as.
    pub type_constraint: Constraint,
}

impl TypeInfo {
    pub fn new(user_type: Maybe<StringId>, type_constraint: Constraint) -> Self {
        Self {
            user_type,
            type_constraint,
        }
    }

    pub fn has_type_constraint(&self) -> bool {
        self.type_constraint.name.is_just()
    }

    /// Be careful, not the same as default()!
    pub fn empty() -> Self {
        Self {
            user_type: Maybe::Nothing,
            type_constraint: Constraint {
                name: Maybe::Just(StringId::EMPTY),
                flags: Default::default(),
            },
        }
    }

    pub fn is_empty(&self) -> bool {
        self.user_type.is_nothing()
            && matches!(
                self.type_constraint,
                Constraint {
                    name: Maybe::Just(StringId::EMPTY),
                    flags: TypeConstraintFlags::NoFlags
                }
            )
    }

    pub fn from_typed_value(v: &TypedValue) -> Self {
        let name = match v {
            TypedValue::Bool(_) => BUILTIN_NAME_BOOL,
            TypedValue::Dict(_) => BUILTIN_NAME_DICT,
            TypedValue::Float(_) => BUILTIN_NAME_FLOAT,
            TypedValue::Int(_) => BUILTIN_NAME_INT,
            TypedValue::Keyset(_) => BUILTIN_NAME_KEYSET,
            TypedValue::LazyClass(_) => BUILTIN_NAME_STRING, // XXX CLASSNAME?
            TypedValue::Null => BUILTIN_NAME_NULL,
            TypedValue::String(_) => BUILTIN_NAME_STRING,
            TypedValue::Uninit => return TypeInfo::empty(),
            TypedValue::Vec(_) => BUILTIN_NAME_VEC,
        };
        let name = crate::intern(name);
        Self {
            user_type: Maybe::Just(name),
            type_constraint: Constraint {
                name: Maybe::Just(name),
                flags: TypeConstraintFlags::NoFlags,
            },
        }
    }
}

impl Default for TypeInfo {
    fn default() -> Self {
        Self::new(Maybe::Just(StringId::EMPTY), Constraint::default())
    }
}

#[derive(Clone, Default, Debug, Eq, PartialEq, Serialize)]
#[repr(C)]
pub struct Constraint {
    pub name: Maybe<StringId>,
    pub flags: TypeConstraintFlags,
}

impl Constraint {
    pub fn new(name: Maybe<StringId>, flags: TypeConstraintFlags) -> Self {
        Self { name, flags }
    }

    pub fn intern(name: impl AsRef<str>, flags: TypeConstraintFlags) -> Self {
        Self {
            name: Maybe::Just(intern::string::intern(name.as_ref())),
            flags,
        }
    }
}

#[derive(Debug, Clone, Eq, PartialEq, Serialize)]
#[repr(C)]
pub struct UpperBound {
    pub name: StringId,
    pub bounds: Vector<TypeInfo>,
}

impl Default for UpperBound {
    fn default() -> Self {
        Self {
            name: StringId::EMPTY,
            bounds: Default::default(),
        }
    }
}

/// As a const fn, given a string removes the leading backslash.
/// r"\HH\AnyArray" -> r"HH\AnyArray".
const fn strip_slash(name: &str) -> &str {
    let parts = name.as_bytes().split_first().unwrap();
    assert!(*parts.0 == b'\\');
    // SAFETY: If we get this far, the input name was nonempty and started with '\'
    // so this cannot split a unicode character in half.
    unsafe { std::str::from_utf8_unchecked(parts.1) }
}

pub static BUILTIN_NAME_ANY_ARRAY: &str = strip_slash(naming_special_names::collections::ANY_ARRAY);
pub static BUILTIN_NAME_ARRAYKEY: &str = strip_slash(naming_special_names::typehints::HH_ARRAYKEY);
pub static BUILTIN_NAME_BOOL: &str = strip_slash(naming_special_names::typehints::HH_BOOL);
pub static BUILTIN_NAME_CLASSNAME: &str = strip_slash(naming_special_names::classes::CLASS_NAME);
pub static BUILTIN_NAME_DARRAY: &str = strip_slash(naming_special_names::typehints::HH_DARRAY);
pub static BUILTIN_NAME_DICT: &str = strip_slash(naming_special_names::collections::DICT);
pub static BUILTIN_NAME_FLOAT: &str = strip_slash(naming_special_names::typehints::HH_FLOAT);
pub static BUILTIN_NAME_INT: &str = strip_slash(naming_special_names::typehints::HH_INT);
pub static BUILTIN_NAME_KEYSET: &str = strip_slash(naming_special_names::collections::KEYSET);
pub static BUILTIN_NAME_NONNULL: &str = strip_slash(naming_special_names::typehints::HH_NONNULL);
pub static BUILTIN_NAME_NORETURN: &str = strip_slash(naming_special_names::typehints::HH_NORETURN);
pub static BUILTIN_NAME_NOTHING: &str = strip_slash(naming_special_names::typehints::HH_NOTHING);
pub static BUILTIN_NAME_NULL: &str = strip_slash(naming_special_names::typehints::HH_NULL);
pub static BUILTIN_NAME_NUM: &str = strip_slash(naming_special_names::typehints::HH_NUM);
pub static BUILTIN_NAME_RESOURCE: &str = strip_slash(naming_special_names::typehints::HH_RESOURCE);
pub static BUILTIN_NAME_STRING: &str = strip_slash(naming_special_names::typehints::HH_STRING);
pub static BUILTIN_NAME_THIS: &str = strip_slash(naming_special_names::typehints::HH_THIS);
pub static BUILTIN_NAME_TYPENAME: &str = strip_slash(naming_special_names::classes::TYPE_NAME);
pub static BUILTIN_NAME_VARRAY: &str = strip_slash(naming_special_names::typehints::HH_VARRAY);
pub static BUILTIN_NAME_VARRAY_OR_DARRAY: &str =
    strip_slash(naming_special_names::typehints::HH_VARRAY_OR_DARRAY);
pub static BUILTIN_NAME_VEC: &str = strip_slash(naming_special_names::collections::VEC);
pub static BUILTIN_NAME_VEC_OR_DICT: &str =
    strip_slash(naming_special_names::typehints::HH_VEC_OR_DICT);
pub static BUILTIN_NAME_VOID: &str = strip_slash(naming_special_names::typehints::HH_VOID);
pub static BUILTIN_NAME_SOFT_VOID: &str = r"@HH\void";

#[cfg(test)]
mod test {
    #[test]
    fn test_constraint_flags_to_string_called_by_hhbc_hhas() {
        use hhvm_types_ffi::ffi::TypeConstraintFlags;
        let typevar_and_soft = TypeConstraintFlags::TypeVar | TypeConstraintFlags::Soft;
        assert_eq!("type_var soft", typevar_and_soft.to_string());
    }
}
