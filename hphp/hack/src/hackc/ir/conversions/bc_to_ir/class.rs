// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc::Class;

pub(crate) fn convert_class(unit: &mut ir::Unit, cls: &Class) {
    unit.classes.push(ir::Class {
        attributes: cls.attributes.clone().into(),
        base: cls.base.into(),
        constants: cls.constants.clone().into(),
        ctx_constants: cls.ctx_constants.clone().into(),
        doc_comment: cls.doc_comment.clone().map(|c| c.into()).into(),
        enum_includes: cls.enum_includes.clone().into(),
        enum_type: cls.enum_type.clone().into(),
        flags: cls.flags,
        implements: cls.implements.clone().into(),
        methods: Default::default(),
        name: cls.name,
        properties: cls.properties.clone().into(),
        requirements: cls.requirements.clone().into(),
        src_loc: ir::SrcLoc::from_span(&cls.span),
        type_constants: cls.type_constants.clone().into(),
        upper_bounds: cls.upper_bounds.clone().into(),
        uses: cls.uses.clone().into(),
    });
}
