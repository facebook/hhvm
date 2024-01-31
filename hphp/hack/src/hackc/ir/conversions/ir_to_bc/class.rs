// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use ffi::Slice;

use crate::convert;
use crate::convert::UnitBuilder;
use crate::strings::StringCache;
use crate::types;

pub(crate) fn convert_class<'a>(
    alloc: &'a bumpalo::Bump,
    unit: &mut UnitBuilder<'a>,
    class: ir::Class<'a>,
    strings: &StringCache<'a>,
) {
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

    let requirements = Vec::from_iter((requirements.iter()).map(
        |ir::class::Requirement { name, kind }| {
            let name = strings.lookup_class_name(*name);
            hhbc::Requirement { name, kind: *kind }
        },
    ));

    let ctx_constants = Vec::from_iter(
        ctx_constants
            .iter()
            .map(|ctx| convert_ctx_constant(alloc, ctx)),
    );

    let enum_includes = Vec::from_iter(
        enum_includes
            .into_iter()
            .map(|id| strings.lookup_class_name(id)),
    );

    let enum_type: Maybe<_> = enum_type
        .as_ref()
        .map(|et| types::convert(et, strings).unwrap())
        .into();

    let type_constants = Vec::from_iter(
        type_constants
            .into_iter()
            .map(|tc| convert_type_constant(tc, strings)),
    );

    let upper_bounds = Slice::fill_iter(
        alloc,
        upper_bounds.iter().map(|(name, tys)| hhbc::UpperBound {
            name: *name,
            bounds: Slice::fill_iter(
                alloc,
                tys.iter().map(|ty| types::convert(ty, strings).unwrap()),
            ),
        }),
    );

    let base = base.map(|base| strings.lookup_class_name(base)).into();

    let implements = Vec::from_iter(
        implements
            .iter()
            .map(|interface| strings.lookup_class_name(*interface)),
    );

    let name = strings.lookup_class_name(name);

    let uses = Vec::from_iter(uses.into_iter().map(|use_| strings.lookup_class_name(use_)));

    let methods = Vec::from_iter(
        methods
            .into_iter()
            .map(|method| crate::func::convert_method(method, strings, &mut unit.adata_cache)),
    );

    let class = hhbc::Class {
        attributes: convert::convert_attributes(attributes, strings),
        base,
        constants: Vec::from_iter(
            constants
                .into_iter()
                .map(|c| crate::constant::convert_hack_constant(c, strings)),
        )
        .into(),
        ctx_constants: ctx_constants.into(),
        doc_comment: doc_comment.into(),
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
        requirements: requirements.into(),
        span: src_loc.to_span(),
        type_constants: type_constants.into(),
        upper_bounds,
        uses: uses.into(),
    };
    unit.classes.push(class);
}

fn convert_property<'a>(src: ir::Property<'a>, strings: &StringCache<'a>) -> hhbc::Property<'a> {
    hhbc::Property {
        name: strings.lookup_prop_name(src.name),
        flags: src.flags,
        attributes: convert::convert_attributes(src.attributes, strings),
        visibility: src.visibility,
        initial_value: src
            .initial_value
            .map(|tv| convert::convert_typed_value(&tv, strings))
            .into(),
        type_info: types::convert(&src.type_info, strings).unwrap(),
        doc_comment: src.doc_comment,
    }
}

fn convert_ctx_constant<'a>(
    alloc: &'a bumpalo::Bump,
    ctx: &ir::CtxConstant<'a>,
) -> hhbc::CtxConstant<'a> {
    hhbc::CtxConstant {
        name: ctx.name,
        recognized: Slice::fill_iter(alloc, ctx.recognized.iter().cloned()),
        unrecognized: Slice::fill_iter(alloc, ctx.unrecognized.iter().cloned()),
        is_abstract: ctx.is_abstract,
    }
}

fn convert_type_constant<'a>(
    tc: ir::TypeConstant<'a>,
    strings: &StringCache<'a>,
) -> hhbc::TypeConstant<'a> {
    hhbc::TypeConstant {
        name: tc.name,
        initializer: tc
            .initializer
            .map(|init| convert::convert_typed_value(&init, strings))
            .into(),
        is_abstract: tc.is_abstract,
    }
}
