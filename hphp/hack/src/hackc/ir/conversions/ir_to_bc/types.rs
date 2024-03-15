// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use hhbc::TypeInfo;

fn convert_type(ty: &ir::TypeInfo) -> TypeInfo {
    TypeInfo {
        user_type: ty.user_type.into(),
        type_constraint: ty.type_constraint.clone(),
    }
}

fn convert_types(tis: &[ir::TypeInfo]) -> Vec<TypeInfo> {
    tis.iter().map(convert_type).collect()
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
