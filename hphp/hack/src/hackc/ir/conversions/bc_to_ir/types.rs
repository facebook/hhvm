// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use itertools::Itertools;

use crate::types;

pub(crate) fn convert_type(ty: &hhbc::TypeInfo) -> ir::TypeInfo {
    ir::TypeInfo {
        user_type: ty.user_type.into(),
        type_constraint: ty.type_constraint.clone(),
    }
}

pub(crate) fn convert_maybe_type(ty: Maybe<&hhbc::TypeInfo>) -> ir::TypeInfo {
    match ty {
        Maybe::Just(ty) => convert_type(ty),
        Maybe::Nothing => ir::TypeInfo::empty(),
    }
}

pub(crate) fn convert_typedef(td: &hhbc::Typedef) -> ir::Typedef {
    let hhbc::Typedef {
        name,
        attributes,
        type_info_union,
        type_structure,
        span,
        attrs,
        case_type,
    } = td;

    let loc = ir::SrcLoc::from_span(span);
    let type_info_union = type_info_union
        .iter()
        .map(types::convert_type)
        .collect_vec();

    ir::Typedef {
        name: *name,
        attributes: attributes.clone().into(),
        type_info_union,
        type_structure: type_structure.clone(),
        loc,
        attrs: *attrs,
        case_type: *case_type,
    }
}
