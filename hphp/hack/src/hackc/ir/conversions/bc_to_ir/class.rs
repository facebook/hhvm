// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Pair;
use ffi::Str;
use hhbc::Class;
use ir::class::TraitReqKind;
use ir::Type;

use crate::convert;
use crate::types;

pub(crate) fn convert_class<'a>(unit: &mut ir::Unit<'a>, cls: &Class<'a>) {
    let constants: Vec<ir::HackConstant<'a>> = cls
        .constants
        .as_ref()
        .iter()
        .map(crate::constant::convert_constant)
        .collect();

    let enum_type: Option<_> = cls.enum_type.as_ref().map(types::convert_type).into();

    let enum_includes: Vec<_> = cls
        .enum_includes
        .iter()
        .map(|name| ir::ClassId::from_hhbc(*name, &mut unit.strings))
        .collect();

    let type_constants = cls
        .type_constants
        .iter()
        .map(convert_type_constant)
        .collect();

    let ctx_constants = cls.ctx_constants.iter().map(convert_ctx_constant).collect();

    let requirements: Vec<(ir::ClassId, TraitReqKind)> = cls
        .requirements
        .as_ref()
        .iter()
        .map(|Pair(clsid, req_kind)| {
            let clsid = ir::ClassId::from_hhbc(*clsid, &mut unit.strings);
            (clsid, *req_kind)
        })
        .collect();

    let upper_bounds: Vec<(Str<'a>, Vec<Type<'a>>)> = cls
        .upper_bounds
        .iter()
        .map(|Pair(name, tys)| {
            let tys: Vec<Type<'a>> = tys.as_ref().iter().map(types::convert_type).collect();
            (*name, tys)
        })
        .collect();

    let attributes = cls
        .attributes
        .as_ref()
        .iter()
        .map(convert::convert_attribute)
        .collect();

    let base = cls
        .base
        .map(|cls| ir::ClassId::from_hhbc(cls, &mut unit.strings))
        .into();

    let implements: Vec<ir::ClassId> = cls
        .implements
        .as_ref()
        .iter()
        .map(|interface| ir::ClassId::from_hhbc(*interface, &mut unit.strings))
        .collect();

    let name = ir::ClassId::from_hhbc(cls.name, &mut unit.strings);

    unit.classes.push(ir::Class {
        attributes,
        base,
        constants,
        ctx_constants,
        doc_comment: cls.doc_comment.into(),
        enum_includes,
        enum_type,
        flags: cls.flags,
        implements,
        methods: Default::default(),
        name,
        properties: cls.properties.as_ref().to_vec(),
        requirements,
        span: cls.span,
        type_constants,
        upper_bounds,
        uses: cls
            .uses
            .iter()
            .map(|use_| ir::ClassId::from_hhbc(*use_, &mut unit.strings))
            .collect(),
    });
}

fn convert_ctx_constant<'a>(ctx: &hhbc::CtxConstant<'a>) -> ir::CtxConstant<'a> {
    ir::CtxConstant {
        name: ctx.name,
        recognized: ctx.recognized.iter().cloned().collect(),
        unrecognized: ctx.unrecognized.iter().cloned().collect(),
        is_abstract: ctx.is_abstract,
    }
}

fn convert_type_constant<'a>(tc: &hhbc::TypeConstant<'a>) -> ir::TypeConstant<'a> {
    ir::TypeConstant {
        name: tc.name,
        initializer: tc.initializer.clone().into(),
        is_abstract: tc.is_abstract,
    }
}
