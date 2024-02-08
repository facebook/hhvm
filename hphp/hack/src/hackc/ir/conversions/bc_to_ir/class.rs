// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc::Class;
use ir::StringInterner;
use itertools::Itertools;

use crate::convert;
use crate::types;

pub(crate) fn convert_class<'a>(unit: &mut ir::Unit<'a>, filename: ir::Filename, cls: &Class<'a>) {
    let constants = cls
        .constants
        .as_ref()
        .iter()
        .map(|c| crate::constant::convert_constant(c, &unit.strings))
        .collect_vec();

    let enum_type = cls
        .enum_type
        .as_ref()
        .map(|ty| types::convert_type(ty, &unit.strings))
        .into_option();

    let enum_includes = cls
        .enum_includes
        .iter()
        .map(|name| ir::ClassId::from_hhbc(*name, &unit.strings))
        .collect_vec();

    let type_constants = cls
        .type_constants
        .iter()
        .map(|c| convert_type_constant(c, &unit.strings))
        .collect();

    let ctx_constants = cls.ctx_constants.iter().map(convert_ctx_constant).collect();

    let requirements = cls
        .requirements
        .as_ref()
        .iter()
        .map(|hhbc::Requirement { name, kind }| {
            let name = ir::ClassId::from_hhbc(*name, &unit.strings);
            ir::class::Requirement { name, kind: *kind }
        })
        .collect_vec();

    let upper_bounds = cls
        .upper_bounds
        .iter()
        .map(|hhbc::UpperBound { name, bounds }| {
            let tys = (bounds.as_ref().iter())
                .map(|ty| types::convert_type(ty, &unit.strings))
                .collect_vec();
            (*name, tys)
        })
        .collect_vec();

    let attributes = cls
        .attributes
        .as_ref()
        .iter()
        .map(|a| convert::convert_attribute(a, &unit.strings))
        .collect_vec();

    let base = cls
        .base
        .map(|cls| ir::ClassId::from_hhbc(cls, &unit.strings))
        .into();

    let implements = cls
        .implements
        .as_ref()
        .iter()
        .map(|interface| ir::ClassId::from_hhbc(*interface, &unit.strings))
        .collect_vec();

    let properties = cls
        .properties
        .as_ref()
        .iter()
        .map(|prop| convert_property(prop, &unit.strings))
        .collect_vec();

    let name = ir::ClassId::from_hhbc(cls.name, &unit.strings);

    unit.classes.push(ir::Class {
        attributes,
        base,
        constants,
        ctx_constants,
        doc_comment: cls.doc_comment.clone().map(|c| c.into()).into(),
        enum_includes,
        enum_type,
        flags: cls.flags,
        implements,
        methods: Default::default(),
        name,
        properties,
        requirements,
        src_loc: ir::SrcLoc::from_span(filename, &cls.span),
        type_constants,
        upper_bounds,
        uses: cls
            .uses
            .iter()
            .map(|use_| ir::ClassId::from_hhbc(*use_, &unit.strings))
            .collect(),
    });
}

fn convert_property(prop: &hhbc::Property<'_>, strings: &ir::StringInterner) -> ir::Property {
    let attributes = prop
        .attributes
        .iter()
        .map(|a| convert::convert_attribute(a, strings))
        .collect_vec();
    ir::Property {
        name: ir::PropId::from_hhbc(prop.name, strings),
        flags: prop.flags,
        attributes,
        visibility: prop.visibility,
        initial_value: prop
            .initial_value
            .as_ref()
            .map(|tv| convert::convert_typed_value(tv, strings))
            .into(),
        type_info: types::convert_type(&prop.type_info, strings),
        doc_comment: prop.doc_comment.clone().map(|c| c.into()),
    }
}

fn convert_ctx_constant<'a>(ctx: &hhbc::CtxConstant<'a>) -> ir::CtxConstant<'a> {
    ir::CtxConstant {
        name: ctx.name,
        recognized: ctx.recognized.iter().cloned().collect(),
        unrecognized: ctx.unrecognized.iter().cloned().collect(),
        is_abstract: ctx.is_abstract,
    }
}

fn convert_type_constant<'a>(
    tc: &hhbc::TypeConstant<'a>,
    strings: &StringInterner,
) -> ir::TypeConstant<'a> {
    let initializer = tc
        .initializer
        .as_ref()
        .map(|tv| convert::convert_typed_value(tv, strings))
        .into();
    ir::TypeConstant {
        name: tc.name,
        initializer,
        is_abstract: tc.is_abstract,
    }
}
