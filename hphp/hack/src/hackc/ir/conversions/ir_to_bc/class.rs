// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use ffi::Pair;
use ffi::Slice;
use ir::class::TraitReqKind;
use ir::string_intern::StringInterner;

use crate::convert;
use crate::convert::HackCUnitBuilder;
use crate::types;

pub(crate) fn convert_class<'a>(
    alloc: &'a bumpalo::Bump,
    unit: &mut HackCUnitBuilder<'a>,
    class: ir::Class<'a>,
    strings: &StringInterner<'a>,
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
            let clsid = clsid.to_hhbc(alloc, strings);
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
            .map(|id| id.to_hhbc(alloc, strings)),
    );

    let enum_type: Maybe<_> = enum_type
        .as_ref()
        .map(|et| types::convert(et).unwrap())
        .into();

    let type_constants = Slice::fill_iter(alloc, type_constants.iter().map(convert_type_constant));

    let upper_bounds = Slice::fill_iter(
        alloc,
        upper_bounds.iter().map(|(name, tys)| {
            Pair(
                *name,
                Slice::fill_iter(alloc, tys.iter().map(|ty| types::convert(ty).unwrap())),
            )
        }),
    );

    let base = base.map(|base| base.to_hhbc(alloc, strings)).into();

    let implements = Slice::fill_iter(
        alloc,
        implements
            .iter()
            .map(|interface| interface.to_hhbc(alloc, strings)),
    );

    let name = name.to_hhbc(alloc, strings);

    let uses = Slice::fill_iter(
        alloc,
        uses.into_iter().map(|use_| use_.to_hhbc(alloc, strings)),
    );

    let class = hhbc::hhas_class::HhasClass {
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
                .map(|method| crate::func::convert_method(alloc, method, strings, unit)),
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
) -> hhbc::hhas_coeffects::HhasCtxConstant<'a> {
    hhbc::hhas_coeffects::HhasCtxConstant {
        name: ctx.name,
        recognized: Slice::fill_iter(alloc, ctx.recognized.iter().cloned()),
        unrecognized: Slice::fill_iter(alloc, ctx.unrecognized.iter().cloned()),
        is_abstract: ctx.is_abstract,
    }
}

fn convert_type_constant<'a>(
    tc: &ir::TypeConstant<'a>,
) -> hhbc::hhas_type_const::HhasTypeConstant<'a> {
    hhbc::hhas_type_const::HhasTypeConstant {
        name: tc.name,
        initializer: tc.initializer.clone().into(),
        is_abstract: tc.is_abstract,
    }
}
