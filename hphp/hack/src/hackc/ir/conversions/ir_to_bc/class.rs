// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;

use crate::convert;
use crate::convert::UnitBuilder;
use crate::strings::StringCache;
use crate::types;

pub(crate) fn convert_class(unit: &mut UnitBuilder, class: ir::Class, strings: &StringCache) {
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

    let ctx_constants = Vec::from_iter(ctx_constants.iter().map(|ctx| convert_ctx_constant(ctx)));

    let enum_type: Maybe<_> = enum_type
        .as_ref()
        .map(|et| types::convert(et, strings).unwrap())
        .into();

    let type_constants = Vec::from_iter(
        type_constants
            .into_iter()
            .map(|tc| convert_type_constant(tc, strings)),
    );

    let upper_bounds = Vec::from_iter(upper_bounds.iter().map(|(name, tys)| hhbc::UpperBound {
        name: *name,
        bounds: Vec::from_iter(tys.iter().map(|ty| types::convert(ty, strings).unwrap())).into(),
    }));

    let methods = Vec::from_iter(
        methods
            .into_iter()
            .map(|method| crate::func::convert_method(method, strings, &mut unit.adata_cache)),
    );

    let class = hhbc::Class {
        attributes: convert::convert_attributes(attributes, strings).into(),
        base: base.into(),
        constants: Vec::from_iter(
            constants
                .into_iter()
                .map(|c| crate::constant::convert_hack_constant(c, strings)),
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
        properties: Vec::from_iter(
            properties
                .into_iter()
                .map(|prop| convert_property(prop, strings)),
        )
        .into(),
        requirements: requirements.clone().into(),
        span: src_loc.to_span(),
        type_constants: type_constants.into(),
        upper_bounds: upper_bounds.into(),
        uses: uses.into(),
    };
    unit.classes.push(class);
}

fn convert_property(src: ir::Property, strings: &StringCache) -> hhbc::Property {
    hhbc::Property {
        name: strings.lookup_prop_name(src.name),
        flags: src.flags,
        attributes: convert::convert_attributes(src.attributes, strings).into(),
        visibility: src.visibility,
        initial_value: src
            .initial_value
            .map(|tv| convert::convert_typed_value(&tv, strings))
            .into(),
        type_info: types::convert(&src.type_info, strings).unwrap(),
        doc_comment: src.doc_comment.map(|c| c.into()),
    }
}

fn convert_ctx_constant(ctx: &ir::CtxConstant) -> hhbc::CtxConstant {
    hhbc::CtxConstant {
        name: ctx.name,
        recognized: ctx.recognized.clone().into(),
        unrecognized: ctx.unrecognized.clone().into(),
        is_abstract: ctx.is_abstract,
    }
}

fn convert_type_constant(tc: ir::TypeConstant, strings: &StringCache) -> hhbc::TypeConstant {
    hhbc::TypeConstant {
        name: tc.name,
        initializer: tc
            .initializer
            .map(|init| convert::convert_typed_value(&init, strings))
            .into(),
        is_abstract: tc.is_abstract,
    }
}
