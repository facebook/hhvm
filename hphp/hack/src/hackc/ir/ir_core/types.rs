// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fmt;
use std::sync::OnceLock;

use maplit::hashmap;

use crate::Attr;
use crate::Attribute;
use crate::ClassName;
use crate::SrcLoc;
use crate::StringId;
use crate::TypeConstraintFlags;
use crate::TypeInfo;
use crate::TypedValue;

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
                .into_option()
                .as_ref()
                .and_then(|user_type| {
                    use std::collections::HashMap;
                    static UNCONSTRAINED_BY_NAME: OnceLock<HashMap<&'static str, BaseType>> =
                        OnceLock::new();
                    let unconstrained_by_name = UNCONSTRAINED_BY_NAME.get_or_init(|| {
                        hashmap! {
                            hhbc::BUILTIN_NAME_VOID => BaseType::Void,
                            hhbc::BUILTIN_NAME_SOFT_VOID => BaseType::Void,
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

    pub fn is_reference_type(&self) -> bool {
        matches!(self.ty, BaseType::Class(_) | BaseType::This)
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

fn cvt_constraint_type(name: StringId) -> BaseType {
    use std::collections::HashMap;
    static CONSTRAINT_BY_NAME: OnceLock<HashMap<&'static str, BaseType>> = OnceLock::new();
    let constraint_by_name = CONSTRAINT_BY_NAME.get_or_init(|| {
        hashmap! {
            hhbc::BUILTIN_NAME_ANY_ARRAY => BaseType::AnyArray,
            hhbc::BUILTIN_NAME_ARRAYKEY => BaseType::Arraykey,
            hhbc::BUILTIN_NAME_BOOL => BaseType::Bool,
            hhbc::BUILTIN_NAME_CLASSNAME => BaseType::Classname,
            hhbc::BUILTIN_NAME_DARRAY => BaseType::Darray,
            hhbc::BUILTIN_NAME_DICT => BaseType::Dict,
            hhbc::BUILTIN_NAME_FLOAT => BaseType::Float,
            hhbc::BUILTIN_NAME_INT => BaseType::Int,
            hhbc::BUILTIN_NAME_KEYSET => BaseType::Keyset,
            hhbc::BUILTIN_NAME_NONNULL => BaseType::Nonnull,
            hhbc::BUILTIN_NAME_NORETURN => BaseType::Noreturn,
            hhbc::BUILTIN_NAME_NOTHING => BaseType::Nothing,
            hhbc::BUILTIN_NAME_NULL => BaseType::Null,
            hhbc::BUILTIN_NAME_NUM => BaseType::Num,
            hhbc::BUILTIN_NAME_RESOURCE => BaseType::Resource,
            hhbc::BUILTIN_NAME_STRING => BaseType::String,
            hhbc::BUILTIN_NAME_THIS => BaseType::This,
            hhbc::BUILTIN_NAME_TYPENAME => BaseType::Typename,
            hhbc::BUILTIN_NAME_VARRAY => BaseType::Varray,
            hhbc::BUILTIN_NAME_VARRAY_OR_DARRAY => BaseType::VarrayOrDarray,
            hhbc::BUILTIN_NAME_VEC => BaseType::Vec,
            hhbc::BUILTIN_NAME_VEC_OR_DICT => BaseType::VecOrDict,
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
