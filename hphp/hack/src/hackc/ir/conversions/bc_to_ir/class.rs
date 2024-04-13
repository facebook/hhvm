// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc::Class;

pub(crate) fn convert_class(cls: Class) -> ir::Class {
    ir::Class {
        attributes: cls.attributes.into(),
        base: cls.base.into(),
        constants: cls.constants.into(),
        ctx_constants: cls.ctx_constants.into(),
        doc_comment: cls.doc_comment.map(|c| c.into()).into(),
        enum_includes: cls.enum_includes.into(),
        enum_type: cls.enum_type.into(),
        flags: cls.flags,
        implements: cls.implements.into(),
        methods: (cls.methods.into_iter())
            .map(crate::func::convert_method)
            .collect(),
        name: cls.name,
        properties: cls.properties.into(),
        requirements: cls.requirements.into(),
        span: cls.span,
        type_constants: cls.type_constants.into(),
        upper_bounds: cls.upper_bounds.into(),
        uses: cls.uses.into(),
    }
}
