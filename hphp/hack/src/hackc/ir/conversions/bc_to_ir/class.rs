// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc::Class;

pub(crate) fn convert_class(cls: Class) -> ir::Class {
    ir::Class {
        attributes: cls.attributes,
        base: cls.base,
        constants: cls.constants,
        ctx_constants: cls.ctx_constants,
        doc_comment: cls.doc_comment,
        enum_includes: cls.enum_includes,
        enum_type: cls.enum_type,
        flags: cls.flags,
        implements: cls.implements,
        methods: (cls.methods.into_iter())
            .map(crate::func::convert_method)
            .collect(),
        name: cls.name,
        properties: cls.properties,
        requirements: cls.requirements,
        span: cls.span,
        type_constants: cls.type_constants,
        upper_bounds: cls.upper_bounds,
        uses: cls.uses,
    }
}
