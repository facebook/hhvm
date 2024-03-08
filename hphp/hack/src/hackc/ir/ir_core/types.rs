// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fmt;

use naming_special_names_rust as naming_special_names;

use crate::Attr;
use crate::Attribute;
use crate::ClassName;
use crate::SrcLoc;
use crate::StringInterner;
use crate::TypeConstraintFlags;
use crate::TypedValue;
use crate::UnitBytesId;

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
    pub fn write(&self, f: &mut fmt::Formatter<'_>, strings: &StringInterner) -> fmt::Result {
        match self {
            BaseType::AnyArray => f.write_str("AnyArray"),
            BaseType::Arraykey => f.write_str("Arraykey"),
            BaseType::Bool => f.write_str("Bool"),
            BaseType::Class(cid) => {
                write!(f, "Class(\"{}\")", strings.display(cid.as_bytes_id()))
            }
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

    pub fn write(&self, f: &mut fmt::Formatter<'_>, strings: &StringInterner) -> fmt::Result {
        f.write_str("Constraint { ty: ")?;
        self.ty.write(f, strings)?;
        f.write_str(", modifiers: ")?;

        if self.modifiers == TypeConstraintFlags::NoFlags {
            f.write_str("none")?;
        } else {
            let mut sep = "";
            let mut check = |flag: TypeConstraintFlags, s: &str| {
                if self.modifiers.contains(flag) {
                    write!(f, "{sep}{s}")?;
                    sep = " | ";
                }
                Ok(())
            };

            check(TypeConstraintFlags::Nullable, "nullable")?;
            check(TypeConstraintFlags::Nullable, "case_type")?;
            check(TypeConstraintFlags::ExtendedHint, "extended_hint")?;
            check(TypeConstraintFlags::TypeVar, "type_var")?;
            check(TypeConstraintFlags::Soft, "soft")?;
            check(TypeConstraintFlags::TypeConstant, "type_constant")?;
            check(TypeConstraintFlags::Resolved, "resolved")?;
            check(TypeConstraintFlags::DisplayNullable, "display_nullable")?;
            check(TypeConstraintFlags::UpperBound, "upper_bound")?;
        }
        f.write_str(" }")
    }

    pub fn is_this(&self) -> bool {
        self.ty.is_this()
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
    pub user_type: Option<UnitBytesId>,
    /// The underlying type this TypeInfo is constrained as.
    pub enforced: EnforceableType,
}

impl TypeInfo {
    pub fn empty() -> Self {
        Self {
            user_type: None,
            enforced: EnforceableType::default(),
        }
    }

    pub fn is_empty(&self) -> bool {
        matches!(
            self,
            TypeInfo {
                user_type: None,
                enforced: EnforceableType {
                    ty: BaseType::None,
                    modifiers: TypeConstraintFlags::NoFlags
                }
            }
        )
    }

    pub fn write(&self, f: &mut fmt::Formatter<'_>, strings: &StringInterner) -> fmt::Result {
        f.write_str("TypeInfo { user_type: ")?;
        if let Some(ut) = self.user_type {
            write!(f, "\"{}\"", strings.display(ut))?;
        } else {
            f.write_str("none")?;
        }
        f.write_str(", constraint: ")?;
        self.enforced.write(f, strings)?;
        f.write_str("}")
    }

    pub fn display<'a>(&'a self, strings: &'a StringInterner) -> impl fmt::Display + 'a {
        struct D<'a> {
            strings: &'a StringInterner,
            self_: &'a TypeInfo,
        }

        impl fmt::Display for D<'_> {
            fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
                self.self_.write(f, self.strings)
            }
        }

        D {
            strings,
            self_: self,
        }
    }
}

impl From<EnforceableType> for TypeInfo {
    fn from(enforced: EnforceableType) -> TypeInfo {
        TypeInfo {
            user_type: None,
            enforced,
        }
    }
}

impl From<BaseType> for TypeInfo {
    fn from(ty: BaseType) -> TypeInfo {
        TypeInfo {
            user_type: None,
            enforced: ty.into(),
        }
    }
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
