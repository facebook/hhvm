// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use ffi::Pair;
use ffi::Slice;
use ir::class::TraitReqKind;

use crate::convert;
use crate::convert::UnitBuilder;
use crate::strings::StringCache;
use crate::types;

pub(crate) fn convert_class<'a>(
    alloc: &'a bumpalo::Bump,
    unit: &mut UnitBuilder<'a>,
    class: ir::Class<'a>,
    strings: &StringCache<'a, '_>,
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
        span,
        type_constants,
        upper_bounds,
        uses,
    } = class;

    let requirements: Slice<'a, Pair<hhbc::ClassName<'a>, TraitReqKind>> = Slice::fill_iter(
        alloc,
        requirements.iter().map(|(clsid, req_kind)| {
            let clsid = strings.lookup_class_name(*clsid);
            Pair(clsid, *req_kind)
        }),
    );

    let ctx_constants = Slice::fill_iter(
        alloc,
        ctx_constants
            .iter()
            .map(|ctx| convert_ctx_constant(alloc, ctx)),
    );

    let enum_includes = Slice::fill_iter(
        alloc,
        enum_includes
            .into_iter()
            .map(|id| strings.lookup_class_name(id)),
    );

    let enum_type: Maybe<_> = enum_type
        .as_ref()
        .map(|et| types::convert(alloc, et, strings).unwrap())
        .into();

    let type_constants = Slice::fill_iter(alloc, type_constants.iter().map(convert_type_constant));

    let upper_bounds = Slice::fill_iter(
        alloc,
        upper_bounds.iter().map(|(name, tys)| {
            Pair(
                *name,
                Slice::fill_iter(
                    alloc,
                    tys.iter()
                        .map(|ty| types::convert(alloc, ty, strings).unwrap()),
                ),
            )
        }),
    );

    let base = base.map(|base| strings.lookup_class_name(base)).into();

    let implements = Slice::fill_iter(
        alloc,
        implements
            .iter()
            .map(|interface| strings.lookup_class_name(*interface)),
    );

    let name = strings.lookup_class_name(name);

    let uses = Slice::fill_iter(
        alloc,
        uses.into_iter().map(|use_| strings.lookup_class_name(use_)),
    );

    let class = hhbc::Class {
        attributes: convert::convert_attributes(alloc, attributes),
        base,
        constants: Slice::fill_iter(
            alloc,
            constants
                .into_iter()
                .map(crate::constant::convert_hack_constant),
        ),
        ctx_constants,
        doc_comment: doc_comment.into(),
        enum_includes,
        enum_type,
        flags,
        implements,
        methods: Slice::fill_iter(
            alloc,
            methods
                .into_iter()
                .map(|method| crate::func::convert_method(alloc, method, strings)),
        ),
        name,
        properties: Slice::fill_iter(alloc, properties.iter().cloned()),
        requirements,
        span,
        type_constants,
        upper_bounds,
        uses,
    };
    unit.classes.push(class);
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

fn convert_type_constant<'a>(tc: &ir::TypeConstant<'a>) -> hhbc::TypeConstant<'a> {
    hhbc::TypeConstant {
        name: tc.name,
        initializer: tc.initializer.clone().into(),
        is_abstract: tc.is_abstract,
    }
}
