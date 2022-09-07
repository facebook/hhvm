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

fn convert_type<'a>(
    alloc: &'a bumpalo::Bump,
    ty: &ir::UserType,
    strings: &StringCache<'a, '_>,
) -> TypeInfo<'a> {
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
                let p = alloc.alloc_slice_fill_copy(len, 0u8);
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
            name: name.map(|name| Str::new_slice(alloc, &name)).into(),
            flags: ty.enforced.modifiers,
        },
    }
}

fn base_type_string(ty: &ir::BaseType) -> Option<Str<'static>> {
    match ty {
        BaseType::Class(_) | BaseType::Mixed | BaseType::None | BaseType::Void => None,
        BaseType::Arraykey => Some(ir::types::BUILTIN_NAME_ARRAYKEY),
        BaseType::Bool => Some(ir::types::BUILTIN_NAME_BOOL),
        BaseType::Dict => Some(ir::types::BUILTIN_NAME_DICT),
        BaseType::Float => Some(ir::types::BUILTIN_NAME_FLOAT),
        BaseType::AnyArray => Some(ir::types::BUILTIN_NAME_ANY_ARRAY),
        BaseType::Nothing => Some(ir::types::BUILTIN_NAME_NOTHING),
        BaseType::Classname => Some(ir::types::BUILTIN_NAME_CLASSNAME),
        BaseType::This => Some(ir::types::BUILTIN_NAME_THIS),
        BaseType::Resource => Some(ir::types::BUILTIN_NAME_RESOURCE),
        BaseType::Num => Some(ir::types::BUILTIN_NAME_NUM),
        BaseType::Int => Some(ir::types::BUILTIN_NAME_INT),
        BaseType::Keyset => Some(ir::types::BUILTIN_NAME_KEYSET),
        BaseType::Typename => Some(ir::types::BUILTIN_NAME_TYPENAME),
        BaseType::Nonnull => Some(ir::types::BUILTIN_NAME_NONNULL),
        BaseType::Noreturn => Some(ir::types::BUILTIN_NAME_NORETURN),
        BaseType::Null => Some(ir::types::BUILTIN_NAME_NULL),
        BaseType::String => Some(ir::types::BUILTIN_NAME_STRING),
        BaseType::Varray => Some(ir::types::BUILTIN_NAME_VARRAY),
        BaseType::Darray => Some(ir::types::BUILTIN_NAME_DARRAY),
        BaseType::Vec => Some(ir::types::BUILTIN_NAME_VEC),
        BaseType::VecOrDict => Some(ir::types::BUILTIN_NAME_VEC_OR_DICT),
        BaseType::VarrayOrDarray => Some(ir::types::BUILTIN_NAME_VARRAY_OR_DARRAY),
    }
}

pub(crate) fn convert<'a>(
    alloc: &'a bumpalo::Bump,
    ty: &ir::UserType,
    strings: &StringCache<'a, '_>,
) -> Maybe<TypeInfo<'a>> {
    if ty.is_empty() {
        Maybe::Nothing
    } else {
        Maybe::Just(convert_type(alloc, ty, strings))
    }
}
