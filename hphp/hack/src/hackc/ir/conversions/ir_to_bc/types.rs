// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use hhbc::Constraint;
use hhbc::TypeInfo;
use ir::BaseType;
use ir::TypeConstraintFlags;

fn convert_type(ty: &ir::TypeInfo) -> TypeInfo {
    let mut user_type = ty.user_type;
    let name = if let Some(name) = base_type_string(&ty.enforced.ty) {
        if user_type.is_none() {
            let nullable = ty
                .enforced
                .modifiers
                .contains(TypeConstraintFlags::Nullable);
            let soft = ty.enforced.modifiers.contains(TypeConstraintFlags::Soft);
            user_type = Some(if !nullable && !soft {
                hhbc::intern(name)
            } else {
                let len = name.len() + nullable as usize + soft as usize;
                let mut p = String::with_capacity(len);
                if soft {
                    p.push('@');
                }
                if nullable {
                    p.push('?');
                }
                p.push_str(name);
                hhbc::intern(p)
            });
        }
        Some(hhbc::intern(name))
    } else {
        match ty.enforced.ty {
            BaseType::Mixed | BaseType::Void => None,
            BaseType::Class(name) => Some(name.as_string_id()),
            _ => unreachable!(),
        }
    };

    TypeInfo {
        user_type: user_type.into(),
        type_constraint: Constraint {
            name: name.into(),
            flags: ty.enforced.modifiers,
        },
    }
}

fn convert_types(tis: &[ir::TypeInfo]) -> Vec<TypeInfo> {
    tis.iter().map(convert_type).collect()
}

fn base_type_string(ty: &ir::BaseType) -> Option<&'static str> {
    match ty {
        BaseType::Class(_) | BaseType::Mixed | BaseType::Void => None,
        BaseType::None => Some(""),
        BaseType::AnyArray => Some(ir::types::BUILTIN_NAME_ANY_ARRAY),
        BaseType::Arraykey => Some(ir::types::BUILTIN_NAME_ARRAYKEY),
        BaseType::Bool => Some(ir::types::BUILTIN_NAME_BOOL),
        BaseType::Classname => Some(ir::types::BUILTIN_NAME_CLASSNAME),
        BaseType::Darray => Some(ir::types::BUILTIN_NAME_DARRAY),
        BaseType::Dict => Some(ir::types::BUILTIN_NAME_DICT),
        BaseType::Float => Some(ir::types::BUILTIN_NAME_FLOAT),
        BaseType::Int => Some(ir::types::BUILTIN_NAME_INT),
        BaseType::Keyset => Some(ir::types::BUILTIN_NAME_KEYSET),
        BaseType::Nonnull => Some(ir::types::BUILTIN_NAME_NONNULL),
        BaseType::Noreturn => Some(ir::types::BUILTIN_NAME_NORETURN),
        BaseType::Nothing => Some(ir::types::BUILTIN_NAME_NOTHING),
        BaseType::Null => Some(ir::types::BUILTIN_NAME_NULL),
        BaseType::Num => Some(ir::types::BUILTIN_NAME_NUM),
        BaseType::Resource => Some(ir::types::BUILTIN_NAME_RESOURCE),
        BaseType::String => Some(ir::types::BUILTIN_NAME_STRING),
        BaseType::This => Some(ir::types::BUILTIN_NAME_THIS),
        BaseType::Typename => Some(ir::types::BUILTIN_NAME_TYPENAME),
        BaseType::Varray => Some(ir::types::BUILTIN_NAME_VARRAY),
        BaseType::VarrayOrDarray => Some(ir::types::BUILTIN_NAME_VARRAY_OR_DARRAY),
        BaseType::Vec => Some(ir::types::BUILTIN_NAME_VEC),
        BaseType::VecOrDict => Some(ir::types::BUILTIN_NAME_VEC_OR_DICT),
    }
}

pub(crate) fn convert(ty: &ir::TypeInfo) -> Maybe<TypeInfo> {
    if ty.is_empty() {
        Maybe::Nothing
    } else {
        Maybe::Just(convert_type(ty))
    }
}

pub(crate) fn convert_typedef(td: ir::Typedef) -> hhbc::Typedef {
    let ir::Typedef {
        name,
        attributes,
        type_info_union,
        type_structure,
        loc,
        attrs,
        case_type,
    } = td;

    let span = hhbc::Span {
        line_begin: loc.line_begin,
        line_end: loc.line_end,
    };
    let attributes = crate::convert::convert_attributes(attributes);
    let type_info_union = convert_types(type_info_union.as_ref());

    hhbc::Typedef {
        name,
        attributes: attributes.into(),
        type_info_union: type_info_union.into(),
        type_structure,
        span,
        attrs,
        case_type,
    }
}
