// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fmt;

use ffi::Str;
pub use hhvm_types_ffi::ffi::TypeConstraintFlags;

use crate::newtype::ClassId;
use crate::StringInterner;
use crate::UnitStringId;

pub static BUILTIN_NAME_ANY_ARRAY: Str<'static> = Str::new(br"HH\AnyArray");
pub static BUILTIN_NAME_ARRAYKEY: Str<'static> = Str::new(br"HH\arraykey");
pub static BUILTIN_NAME_BOOL: Str<'static> = Str::new(br"HH\bool");
pub static BUILTIN_NAME_CLASSNAME: Str<'static> = Str::new(br"HH\classname");
pub static BUILTIN_NAME_DARRAY: Str<'static> = Str::new(br"HH\darray");
pub static BUILTIN_NAME_DICT: Str<'static> = Str::new(br"HH\dict");
pub static BUILTIN_NAME_FLOAT: Str<'static> = Str::new(br"HH\float");
pub static BUILTIN_NAME_INT: Str<'static> = Str::new(br"HH\int");
pub static BUILTIN_NAME_KEYSET: Str<'static> = Str::new(br"HH\keyset");
pub static BUILTIN_NAME_NONNULL: Str<'static> = Str::new(br"HH\nonnull");
pub static BUILTIN_NAME_NORETURN: Str<'static> = Str::new(br"HH\noreturn");
pub static BUILTIN_NAME_NOTHING: Str<'static> = Str::new(br"HH\nothing");
pub static BUILTIN_NAME_NULL: Str<'static> = Str::new(br"HH\null");
pub static BUILTIN_NAME_NUM: Str<'static> = Str::new(br"HH\num");
pub static BUILTIN_NAME_RESOURCE: Str<'static> = Str::new(br"HH\resource");
pub static BUILTIN_NAME_STRING: Str<'static> = Str::new(br"HH\string");
pub static BUILTIN_NAME_THIS: Str<'static> = Str::new(br"HH\this");
pub static BUILTIN_NAME_TYPENAME: Str<'static> = Str::new(br"HH\typename");
pub static BUILTIN_NAME_VARRAY: Str<'static> = Str::new(br"HH\varray");
pub static BUILTIN_NAME_VARRAY_OR_DARRAY: Str<'static> = Str::new(br"HH\varray_or_darray");
pub static BUILTIN_NAME_VEC: Str<'static> = Str::new(br"HH\vec");
pub static BUILTIN_NAME_VEC_OR_DICT: Str<'static> = Str::new(br"HH\vec_or_dict");
pub static BUILTIN_NAME_VOID: Str<'static> = Str::new(br"HH\void");
pub static BUILTIN_NAME_SOFT_VOID: Str<'static> = Str::new(br"@HH\void");

#[derive(Clone, Debug, PartialEq, Eq, Hash)]
pub enum BaseType {
    AnyArray,
    Arraykey,
    Bool,
    Class(ClassId),
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
    pub fn write(&self, f: &mut fmt::Formatter<'_>, strings: &StringInterner<'_>) -> fmt::Result {
        match self {
            BaseType::AnyArray => f.write_str("AnyArray"),
            BaseType::Arraykey => f.write_str("Arraykey"),
            BaseType::Bool => f.write_str("Bool"),
            BaseType::Class(cid) => {
                write!(f, "Class(\"{}\")", strings.lookup(cid.id).as_bstr())
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
///
#[derive(Debug, Clone, Eq, PartialEq, Hash)]
pub struct EnforceableType {
    pub ty: BaseType,
    pub modifiers: TypeConstraintFlags,
}

impl EnforceableType {
    pub fn write(&self, f: &mut fmt::Formatter<'_>, strings: &StringInterner<'_>) -> fmt::Result {
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
            check(TypeConstraintFlags::ExtendedHint, "extended_hint")?;
            check(TypeConstraintFlags::TypeVar, "type_var")?;
            check(TypeConstraintFlags::Soft, "soft")?;
            check(TypeConstraintFlags::TypeConstant, "type_constant")?;
            check(TypeConstraintFlags::Resolved, "resolved")?;
            check(TypeConstraintFlags::NoMockObjects, "no_mock_objects")?;
            check(TypeConstraintFlags::DisplayNullable, "display_nullable")?;
            check(TypeConstraintFlags::UpperBound, "upper_bound")?;
        }
        f.write_str(" }")
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

/// A UserType represents a type written by the user.  It consists of the type
/// written by the user (including generics) and an enforced constraint.
#[derive(Clone, Debug, Default)]
pub struct UserType {
    /// The textual type that the user wrote including generics and special
    /// chars (like '?').  If None then this is directly computable from the
    /// enforced type.
    pub user_type: Option<UnitStringId>,
    /// The underlying type this UserType is constrained as.
    pub enforced: EnforceableType,
}

impl UserType {
    pub fn empty() -> Self {
        Self {
            user_type: None,
            enforced: EnforceableType::default(),
        }
    }

    pub fn is_empty(&self) -> bool {
        matches!(
            self,
            UserType {
                user_type: None,
                enforced: EnforceableType {
                    ty: BaseType::None,
                    modifiers: TypeConstraintFlags::NoFlags
                }
            }
        )
    }

    pub fn write(&self, f: &mut fmt::Formatter<'_>, strings: &StringInterner<'_>) -> fmt::Result {
        f.write_str("UserType { user_type: ")?;
        if let Some(ut) = self.user_type {
            write!(f, "\"{}\"", strings.lookup(ut).as_bstr())?;
        } else {
            f.write_str("none")?;
        }
        f.write_str(", constraint: ")?;
        self.enforced.write(f, strings)?;
        f.write_str("}")
    }

    pub fn display<'a>(&'a self, strings: &'a StringInterner<'a>) -> impl fmt::Display + 'a {
        struct D<'a> {
            strings: &'a StringInterner<'a>,
            self_: &'a UserType,
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
