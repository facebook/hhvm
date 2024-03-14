// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;

use crate::convert::UnitBuilder;
use crate::types;

pub(crate) fn convert_class(unit: &mut UnitBuilder, class: ir::Class) {
    let ir::Class {
        attributes,
        base,
        constants,
        ctx_constants,
        doc_comment,
        enum_type,
        enum_includes,
        flags,
        implements,
        methods,
        name,
        properties,
        requirements,
        src_loc,
        type_constants,
        upper_bounds,
        uses,
    } = class;

    let enum_type: Maybe<_> = enum_type
        .as_ref()
        .map(|et| types::convert(et).unwrap())
        .into();

    let upper_bounds = Vec::from_iter(upper_bounds.iter().map(|(name, tys)| hhbc::UpperBound {
        name: *name,
        bounds: Vec::from_iter(tys.iter().map(|ty| types::convert(ty).unwrap())).into(),
    }));

    let methods = Vec::from_iter(
        methods
            .into_iter()
            .map(|method| crate::func::convert_method(method, &mut unit.adata_cache)),
    );

    let class = hhbc::Class {
        attributes: attributes.into(),
        base: base.into(),
        constants: Vec::from_iter(
            constants
                .into_iter()
                .map(crate::constant::convert_hack_constant),
        )
        .into(),
        ctx_constants: ctx_constants.into(),
        doc_comment: doc_comment.map(|c| c.into()).into(),
        enum_includes: enum_includes.into(),
        enum_type,
        flags,
        implements: implements.into(),
        methods: methods.into(),
        name,
        properties: Vec::from_iter(properties.into_iter().map(convert_property)).into(),
        requirements: requirements.into(),
        span: src_loc.to_span(),
        type_constants: type_constants.into(),
        upper_bounds: upper_bounds.into(),
        uses: uses.into(),
    };
    unit.classes.push(class);
}

fn convert_property(src: ir::Property) -> hhbc::Property {
    hhbc::Property {
        name: src.name,
        flags: src.flags,
        attributes: src.attributes.into(),
        visibility: src.visibility,
        initial_value: src.initial_value.into(),
        type_info: types::convert(&src.type_info).unwrap(),
        doc_comment: src.doc_comment.map(|c| c.into()),
    }
}
