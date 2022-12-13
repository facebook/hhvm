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

pub(crate) fn lower_func<'a>(
    mut func: Func<'a>,
    method_info: Option<Arc<MethodInfo<'_>>>,
    strings: Arc<StringInterner>,
) -> Func<'a> {
    trace!(
        "Before Lower: {}",
        ir::print::DisplayFunc(&func, true, &strings)
    );

    // Start by 'unasync'ing the Func.
    ir::passes::unasync(&mut func);
    trace!(
        "After unasync: {}",
        ir::print::DisplayFunc(&func, true, &strings)
    );

    let mut builder = FuncBuilder::with_func(func, Arc::clone(&strings));

    // Simplify various Instrs.
    super::instrs::lower_instrs(&mut builder, method_info);

    // Write the complex constants out as a prelude to the function.
    super::constants::write_constants(&mut builder);

    let mut func = builder.finish();

    ir::passes::split_critical_edges(&mut func, true);

    ir::passes::clean::run(&mut func);

    trace!(
        "After Lower: {}",
        ir::print::DisplayFunc(&func, true, &strings)
    );

    func
}
