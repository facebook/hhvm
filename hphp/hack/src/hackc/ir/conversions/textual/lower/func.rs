// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::Arc;

use ir::Func;
use ir::FuncBuilder;
use ir::StringInterner;
use log::trace;

use crate::func::MethodInfo;
use crate::lower::types::lower_ty;
use crate::lower::types::lower_ty_in_place;

pub(crate) fn lower_func<'a>(
    func: Func<'a>,
    method_info: Option<&MethodInfo<'_>>,
    strings: Arc<StringInterner>,
) -> Func<'a> {
    trace!(
        "Before Lower: {}",
        ir::print::DisplayFunc(&func, true, &strings)
    );

    let mut builder = FuncBuilder::with_func(func, Arc::clone(&strings));

    // Simplify various Instrs.
    super::instrs::lower_instrs(&mut builder, method_info);

    let mut func = builder.finish();
    ir::passes::split_critical_edges(&mut func, true);

    ir::passes::clean::run(&mut func);

    // Lower param types after instrs so that if the instrs refer to the param
    // types it can see the unmolested ones.
    for param in &mut func.params {
        lower_ty_in_place(&mut param.ty, &strings);
    }

    func.return_type = lower_ty(func.return_type, &strings);

    trace!(
        "After Lower: {}",
        ir::print::DisplayFunc(&func, true, &strings)
    );

    func
}
