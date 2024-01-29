// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use ffi::Str;
use hhbc::Constraint;
use hhbc::TypeInfo;
use ir::BaseType;
use ir::TypeConstraintFlags;

use crate::strings::StringCache;

fn convert_type<'a>(ty: &ir::TypeInfo, strings: &StringCache<'a>) -> TypeInfo<'a> {
    let mut user_type = ty.user_type.map(|ut| strings.lookup_ffi_str(ut));

    let name = if let Some(name) = base_type_string(&ty.enforced.ty) {
        if user_type.is_none() {
            let nullable = ty
                .enforced
                .modifiers
                .contains(TypeConstraintFlags::Nullable);
            let soft = ty.enforced.modifiers.contains(TypeConstraintFlags::Soft);
            user_type = Some(if !nullable && !soft {
                name
            } else {
                let len = name.len() + nullable as usize + soft as usize;
                let p = strings.alloc.alloc_slice_fill_copy(len, 0u8);
                let mut i = 0;
                if soft {
                    p[i] = b'@';
                    i += 1;
                }
                if nullable {
                    p[i] = b'?';
                    i += 1;
                }
                p[i..].copy_from_slice(&name);
                ffi::Slice::new(p)
            });
        }
        Some(name)
    } else {
        match ty.enforced.ty {
            BaseType::Mixed | BaseType::Void => None,
            BaseType::Class(name) => Some(strings.lookup_ffi_str(name.id)),
            _ => unreachable!(),
        }
    };

    TypeInfo {
        user_type: user_type.into(),
        type_constraint: Constraint {
            name: name.map(|name| Str::new_slice(strings.alloc, &name)).into(),
            flags: ty.enforced.modifiers,
        },
    }
}

fn convert_types<'a>(tis: &[ir::TypeInfo], strings: &StringCache<'a>) -> Vec<TypeInfo<'a>> {
    tis.iter().map(|ti| convert_type(ti, strings)).collect()
}

fn base_type_string(ty: &ir::BaseType) -> Option<Str<'static>> {
    match ty {
        BaseType::Class(_) | BaseType::Mixed | BaseType::Void => None,
        BaseType::None => Some(Str::new("".as_bytes())),
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

pub(crate) fn convert<'a>(ty: &ir::TypeInfo, strings: &StringCache<'a>) -> Maybe<TypeInfo<'a>> {
    if ty.is_empty() {
        Maybe::Nothing
    } else {
        Maybe::Just(convert_type(ty, strings))
    }
}

pub(crate) fn convert_typedef<'a>(td: ir::Typedef, strings: &StringCache<'a>) -> hhbc::Typedef<'a> {
    let ir::Typedef {
        name,
        attributes,
        type_info_union,
        type_structure,
        loc,
        attrs,
        case_type,
    } = td;

    let name = strings.lookup_class_name(name);
    let span = hhbc::Span {
        line_begin: loc.line_begin,
        line_end: loc.line_end,
    };
    let attributes = crate::convert::convert_attributes(attributes, strings);
    let type_info_union = convert_types(type_info_union.as_ref(), strings);
    let type_structure = crate::convert::convert_typed_value(&type_structure, strings);

    hhbc::Typedef {
        name,
        attributes,
        type_info_union: type_info_union.into(),
        type_structure,
        span,
        attrs,
        case_type,
    }
}
