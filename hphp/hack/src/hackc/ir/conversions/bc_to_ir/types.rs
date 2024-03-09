// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::OnceLock;

use ffi::Maybe;
use hhbc::StringId;
use ir::BaseType;
use ir::StringInterner;
use itertools::Itertools;
use maplit::hashmap;

use crate::convert;
use crate::types;

pub(crate) fn convert_type(ty: &hhbc::TypeInfo) -> ir::TypeInfo {
    let user_type = ty.user_type.into_option();
    let name = ty.type_constraint.name.into_option();
    let constraint_ty = match name {
        // Checking for emptiness to filter out cases where the type constraint is not enforceable
        // (e.g. name = "", hint = type_const).
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
                        ir::types::BUILTIN_NAME_VOID => BaseType::Void,
                        ir::types::BUILTIN_NAME_SOFT_VOID => BaseType::Void,
                    }
                });

                unconstrained_by_name.get(user_type.as_str()).cloned()
            })
            .unwrap_or(BaseType::Mixed),
    };

    ir::TypeInfo {
        user_type,
        enforced: ir::EnforceableType {
            ty: constraint_ty,
            modifiers: ty.type_constraint.flags,
        },
    }
}

pub(crate) fn convert_maybe_type(ty: Maybe<&hhbc::TypeInfo>) -> ir::TypeInfo {
    match ty {
        Maybe::Just(ty) => convert_type(ty),
        Maybe::Nothing => ir::TypeInfo::empty(),
    }
}

fn cvt_constraint_type(name: StringId) -> BaseType {
    use std::collections::HashMap;
    static CONSTRAINT_BY_NAME: OnceLock<HashMap<&'static str, BaseType>> = OnceLock::new();
    let constraint_by_name = CONSTRAINT_BY_NAME.get_or_init(|| {
        hashmap! {
            ir::types::BUILTIN_NAME_ANY_ARRAY => BaseType::AnyArray,
            ir::types::BUILTIN_NAME_ARRAYKEY => BaseType::Arraykey,
            ir::types::BUILTIN_NAME_BOOL => BaseType::Bool,
            ir::types::BUILTIN_NAME_CLASSNAME => BaseType::Classname,
            ir::types::BUILTIN_NAME_DARRAY => BaseType::Darray,
            ir::types::BUILTIN_NAME_DICT => BaseType::Dict,
            ir::types::BUILTIN_NAME_FLOAT => BaseType::Float,
            ir::types::BUILTIN_NAME_INT => BaseType::Int,
            ir::types::BUILTIN_NAME_KEYSET => BaseType::Keyset,
            ir::types::BUILTIN_NAME_NONNULL => BaseType::Nonnull,
            ir::types::BUILTIN_NAME_NORETURN => BaseType::Noreturn,
            ir::types::BUILTIN_NAME_NOTHING => BaseType::Nothing,
            ir::types::BUILTIN_NAME_NULL => BaseType::Null,
            ir::types::BUILTIN_NAME_NUM => BaseType::Num,
            ir::types::BUILTIN_NAME_RESOURCE => BaseType::Resource,
            ir::types::BUILTIN_NAME_STRING => BaseType::String,
            ir::types::BUILTIN_NAME_THIS => BaseType::This,
            ir::types::BUILTIN_NAME_TYPENAME => BaseType::Typename,
            ir::types::BUILTIN_NAME_VARRAY => BaseType::Varray,
            ir::types::BUILTIN_NAME_VARRAY_OR_DARRAY => BaseType::VarrayOrDarray,
            ir::types::BUILTIN_NAME_VEC => BaseType::Vec,
            ir::types::BUILTIN_NAME_VEC_OR_DICT => BaseType::VecOrDict,
        }
    });

    constraint_by_name
        .get(name.as_str())
        .copied()
        .unwrap_or_else(|| BaseType::Class(ir::ClassName::new(name)))
}

pub(crate) fn convert_typedef<'a>(
    td: &hhbc::Typedef,
    filename: ir::Filename,
    strings: &StringInterner,
) -> ir::Typedef {
    let hhbc::Typedef {
        name,
        attributes,
        type_info_union,
        type_structure,
        span,
        attrs,
        case_type,
    } = td;

    let loc = ir::SrcLoc::from_span(filename, span);
    let attributes = attributes
        .iter()
        .map(|a| convert::convert_attribute(a, strings))
        .collect_vec();
    let type_info_union = type_info_union
        .iter()
        .map(types::convert_type)
        .collect_vec();
    let type_structure = convert::convert_typed_value(type_structure, strings);

    ir::Typedef {
        name: *name,
        attributes,
        type_info_union,
        type_structure,
        loc,
        attrs: *attrs,
        case_type: *case_type,
    }
}
