// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::convert::UnitBuilder;

pub(crate) fn convert_class(unit: &mut UnitBuilder, class: ir::Class) {
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

    let methods = Vec::from_iter(
        methods
            .into_iter()
            .map(|method| crate::func::convert_method(method, &mut unit.adata_cache)),
    );

    let class = hhbc::Class {
        attributes,
        base,
        constants,
        ctx_constants,
        doc_comment,
        enum_includes,
        enum_type,
        flags,
        implements,
        methods: methods.into(),
        name,
        properties,
        requirements,
        span,
        type_constants,
        upper_bounds,
        uses,
    };
    unit.classes.push(class);
}
