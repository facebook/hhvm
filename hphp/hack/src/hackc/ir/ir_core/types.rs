// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fmt;
use std::sync::OnceLock;

use ffi::Maybe;
use maplit::hashmap;
use naming_special_names_rust as naming_special_names;

use crate::Attr;
use crate::Attribute;
use crate::ClassName;
use crate::Constraint;
use crate::SrcLoc;
use crate::StringId;
use crate::TypeConstraintFlags;
use crate::TypedValue;

/// As a const fn, given a string removes the leading backslash.
/// r"\HH\AnyArray" -> r"HH\AnyArray".
const fn strip_slash(name: &str) -> &str {
    let parts = name.as_bytes().split_first().unwrap();
    assert!(*parts.0 == b'\\');
    // SAFETY: If we get this far, the input name was nonempty and started with '\'
    // so this cannot split a unicode charater in half.
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

#[derive(Copy, Clone, Debug, PartialEq, Eq, Hash)]
pub enum BaseType {
    AnyArray,
    Arraykey,
    Bool,
    Class(ClassName),
    Classname,
    Darray,
    Dict,
    Float,
    Int,
    Keyset,
    Mixed,
    None,
    Nonnull,
    Noreturn,
    Nothing,
    Null,
    Num,
    Resource,
    String,
    This,
    Typename,
    Varray,
    VarrayOrDarray,
    Vec,
    VecOrDict,
    Void,
}

impl BaseType {
    pub fn write(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            BaseType::AnyArray => f.write_str("AnyArray"),
            BaseType::Arraykey => f.write_str("Arraykey"),
            BaseType::Bool => f.write_str("Bool"),
            BaseType::Class(cid) => write!(f, "Class(\"{cid}\")"),
            BaseType::Classname => f.write_str("Classname"),
            BaseType::Darray => f.write_str("Darray"),
            BaseType::Dict => f.write_str("Dict"),
            BaseType::Float => f.write_str("Float"),
            BaseType::Int => f.write_str("Int"),
            BaseType::Keyset => f.write_str("Keyset"),
            BaseType::Mixed => f.write_str("Mixed"),
            BaseType::None => f.write_str("None"),
            BaseType::Nonnull => f.write_str("Nonnull"),
            BaseType::Noreturn => f.write_str("Noreturn"),
            BaseType::Nothing => f.write_str("Nothing"),
            BaseType::Null => f.write_str("Null"),
            BaseType::Num => f.write_str("Num"),
            BaseType::Resource => f.write_str("Resource"),
            BaseType::String => f.write_str("String"),
            BaseType::This => f.write_str("This"),
            BaseType::Typename => f.write_str("Typename"),
            BaseType::Varray => f.write_str("Varray"),
            BaseType::VarrayOrDarray => f.write_str("VarrayOrDarray"),
            BaseType::Vec => f.write_str("Vec"),
            BaseType::VecOrDict => f.write_str("VecOrDict"),
            BaseType::Void => f.write_str("Void"),
        }
    }

    pub fn is_this(&self) -> bool {
        match self {
            BaseType::This => true,
            _ => false,
        }
    }
}

/// A basic type that is enforced by the underlying Hack runtime.
///
/// Examples:
///   Shapes are only enforcable as a darray - so a parameter which is specified
///   as "shape('a' => int)" would have:
///       ty: BaseType::Darray,
///       modifiers: TypeConstraintFlags::ExtendedHint
///
///   Nullable and int are fully enforcable - so a parameter which is specified
///   as "?int" would have:
///       ty: BaseType::Int,
///       modifiers: TypeConstraintFlags::ExtendedHint | TypeConstraintFlags::Nullable
#[derive(Debug, Clone, Eq, PartialEq, Hash)]
pub struct EnforceableType {
    pub ty: BaseType,
    pub modifiers: TypeConstraintFlags,
}

impl EnforceableType {
    pub fn null() -> Self {
        EnforceableType {
            ty: BaseType::Null,
            modifiers: TypeConstraintFlags::NoFlags,
        }
    }

    pub fn is_this(&self) -> bool {
        self.ty.is_this()
    }

    pub fn from_type_info(ty: &TypeInfo) -> Self {
        let user_type = ty.user_type;
        let name = ty.type_constraint.name;
        let constraint_ty = match name.into_option() {
            // Checking for emptiness to filter out cases where the type constraint
            // is not enforceable, (e.g. name = "", hint = type_const).
            Some(name) if !name.is_empty() => cvt_constraint_type(name),
            Some(_) => BaseType::None,
            _ => user_type
                .as_ref()
                .and_then(|user_type| {
                    use std::collections::HashMap;
                    static UNCONSTRAINED_BY_NAME: OnceLock<HashMap<&'static str, BaseType>> =
                        OnceLock::new();
                    let unconstrained_by_name = UNCONSTRAINED_BY_NAME.get_or_init(|| {
                        hashmap! {
                            BUILTIN_NAME_VOID => BaseType::Void,
                            BUILTIN_NAME_SOFT_VOID => BaseType::Void,
                        }
                    });

                    unconstrained_by_name.get(user_type.as_str()).cloned()
                })
                .unwrap_or(BaseType::Mixed),
        };

        Self {
            ty: constraint_ty,
            modifiers: ty.type_constraint.flags,
        }
    }
}

impl Default for EnforceableType {
    fn default() -> Self {
        Self {
            ty: BaseType::None,
            modifiers: TypeConstraintFlags::NoFlags,
        }
    }
}

impl From<BaseType> for EnforceableType {
    fn from(ty: BaseType) -> Self {
        EnforceableType {
            ty,
            modifiers: TypeConstraintFlags::NoFlags,
        }
    }
}

/// A TypeInfo represents a type written by the user.  It consists of the type
/// written by the user (including generics) and an enforced constraint.
#[derive(Clone, Debug, Default)]
pub struct TypeInfo {
    /// The textual type that the user wrote including generics and special
    /// chars (like '?').  If None then this is directly computable from the
    /// enforced type.
    pub user_type: Option<StringId>,

    /// The underlying type this TypeInfo is constrained as.
    pub type_constraint: Constraint,
}

impl TypeInfo {
    pub fn empty() -> Self {
        Self {
            user_type: None,
            type_constraint: Constraint {
                name: Maybe::Just(StringId::EMPTY),
                flags: Default::default(),
            },
        }
    }

    pub fn is_empty(&self) -> bool {
        self.user_type.is_none()
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
            user_type: Some(name),
            type_constraint: Constraint {
                name: Maybe::Just(name),
                flags: TypeConstraintFlags::NoFlags,
            },
        }
    }

    pub fn is_reference_type(&self) -> bool {
        matches!(
            EnforceableType::from_type_info(self).ty,
            BaseType::Class(_) | BaseType::This
        )
    }
}

fn cvt_constraint_type(name: StringId) -> BaseType {
    use std::collections::HashMap;
    static CONSTRAINT_BY_NAME: OnceLock<HashMap<&'static str, BaseType>> = OnceLock::new();
    let constraint_by_name = CONSTRAINT_BY_NAME.get_or_init(|| {
        hashmap! {
            BUILTIN_NAME_ANY_ARRAY => BaseType::AnyArray,
            BUILTIN_NAME_ARRAYKEY => BaseType::Arraykey,
            BUILTIN_NAME_BOOL => BaseType::Bool,
            BUILTIN_NAME_CLASSNAME => BaseType::Classname,
            BUILTIN_NAME_DARRAY => BaseType::Darray,
            BUILTIN_NAME_DICT => BaseType::Dict,
            BUILTIN_NAME_FLOAT => BaseType::Float,
            BUILTIN_NAME_INT => BaseType::Int,
            BUILTIN_NAME_KEYSET => BaseType::Keyset,
            BUILTIN_NAME_NONNULL => BaseType::Nonnull,
            BUILTIN_NAME_NORETURN => BaseType::Noreturn,
            BUILTIN_NAME_NOTHING => BaseType::Nothing,
            BUILTIN_NAME_NULL => BaseType::Null,
            BUILTIN_NAME_NUM => BaseType::Num,
            BUILTIN_NAME_RESOURCE => BaseType::Resource,
            BUILTIN_NAME_STRING => BaseType::String,
            BUILTIN_NAME_THIS => BaseType::This,
            BUILTIN_NAME_TYPENAME => BaseType::Typename,
            BUILTIN_NAME_VARRAY => BaseType::Varray,
            BUILTIN_NAME_VARRAY_OR_DARRAY => BaseType::VarrayOrDarray,
            BUILTIN_NAME_VEC => BaseType::Vec,
            BUILTIN_NAME_VEC_OR_DICT => BaseType::VecOrDict,
        }
    });

    constraint_by_name
        .get(name.as_str())
        .copied()
        .unwrap_or_else(|| BaseType::Class(ClassName::new(name)))
}

#[derive(Clone, Debug)]
pub struct Typedef {
    pub name: ClassName,
    pub attributes: Vec<Attribute>,
    pub type_info_union: Vec<TypeInfo>,
    pub type_structure: TypedValue,
    pub loc: SrcLoc,
    pub attrs: Attr,
    pub case_type: bool,
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_ti_default() {
        assert_eq!(
            EnforceableType::default(),
            EnforceableType::from_type_info(&TypeInfo::empty()),
        );
    }
}
