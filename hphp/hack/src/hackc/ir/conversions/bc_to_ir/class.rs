// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc::Class;
use itertools::Itertools;

use crate::convert;
use crate::types;

pub(crate) fn convert_class(unit: &mut ir::Unit, cls: &Class) {
    let constants = cls
        .constants
        .as_ref()
        .iter()
        .map(crate::constant::convert_constant)
        .collect_vec();

    let enum_type = cls
        .enum_type
        .as_ref()
        .map(types::convert_type)
        .into_option();

    let type_constants = cls
        .type_constants
        .iter()
        .map(convert_type_constant)
        .collect();

    let ctx_constants = cls.ctx_constants.iter().map(convert_ctx_constant).collect();

    let upper_bounds = cls
        .upper_bounds
        .iter()
        .map(|hhbc::UpperBound { name, bounds }| {
            let tys = (bounds.as_ref().iter())
                .map(types::convert_type)
                .collect_vec();
            (*name, tys)
        })
        .collect_vec();

    let attributes = cls
        .attributes
        .as_ref()
        .iter()
        .map(convert::convert_attribute)
        .collect_vec();

    let properties = cls
        .properties
        .as_ref()
        .iter()
        .map(convert_property)
        .collect_vec();

    unit.classes.push(ir::Class {
        attributes,
        base: cls.base.into(),
        constants,
        ctx_constants,
        doc_comment: cls.doc_comment.clone().map(|c| c.into()).into(),
        enum_includes: cls.enum_includes.clone().into(),
        enum_type,
        flags: cls.flags,
        implements: cls.implements.clone().into(),
        methods: Default::default(),
        name: cls.name,
        properties,
        requirements: cls.requirements.clone().into(),
        src_loc: ir::SrcLoc::from_span(&cls.span),
        type_constants,
        upper_bounds,
        uses: cls.uses.clone().into(),
    });
}

fn convert_property(prop: &hhbc::Property) -> ir::Property {
    let attributes = prop
        .attributes
        .iter()
        .map(convert::convert_attribute)
        .collect_vec();
    ir::Property {
        name: prop.name,
        flags: prop.flags,
        attributes,
        visibility: prop.visibility,
        initial_value: prop
            .initial_value
            .as_ref()
            .map(convert::convert_typed_value)
            .into(),
        type_info: types::convert_type(&prop.type_info),
        doc_comment: prop.doc_comment.clone().map(|c| c.into()),
    }
}

fn convert_ctx_constant(ctx: &hhbc::CtxConstant) -> ir::CtxConstant {
    ir::CtxConstant {
        name: ctx.name,
        recognized: ctx.recognized.clone(),
        unrecognized: ctx.unrecognized.clone(),
        is_abstract: ctx.is_abstract,
    }
}

fn convert_type_constant(tc: &hhbc::TypeConstant) -> ir::TypeConstant {
    let initializer = tc
        .initializer
        .as_ref()
        .map(convert::convert_typed_value)
        .into();
    ir::TypeConstant {
        name: tc.name,
        initializer,
        is_abstract: tc.is_abstract,
    }
}
