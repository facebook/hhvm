// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc::Class;
use itertools::Itertools;

use crate::types;

pub(crate) fn convert_class(unit: &mut ir::Unit, cls: &Class) {
    let enum_type = cls
        .enum_type
        .as_ref()
        .map(types::convert_type)
        .into_option();

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

    let properties = cls
        .properties
        .as_ref()
        .iter()
        .map(convert_property)
        .collect_vec();

    unit.classes.push(ir::Class {
        attributes: cls.attributes.clone().into(),
        base: cls.base.into(),
        constants: cls.constants.clone().into(),
        ctx_constants: cls.ctx_constants.clone().into(),
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
        type_constants: cls.type_constants.clone().into(),
        upper_bounds,
        uses: cls.uses.clone().into(),
    });
}

fn convert_property(prop: &hhbc::Property) -> ir::Property {
    ir::Property {
        name: prop.name,
        flags: prop.flags,
        attributes: prop.attributes.clone().into(),
        visibility: prop.visibility,
        initial_value: prop.initial_value.clone().into(),
        type_info: types::convert_type(&prop.type_info),
        doc_comment: prop.doc_comment.clone().map(|c| c.into()),
    }
}
