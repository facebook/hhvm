// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;

pub(crate) fn convert_maybe_type(ty: Maybe<&hhbc::TypeInfo>) -> ir::TypeInfo {
    match ty {
        Maybe::Just(ty) => ty.clone(),
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

    ir::Typedef {
        name: *name,
        attributes: attributes.clone().into(),
        type_info_union: type_info_union.clone().into(),
        type_structure: type_structure.clone(),
        loc,
        attrs: *attrs,
        case_type: *case_type,
    }
}
