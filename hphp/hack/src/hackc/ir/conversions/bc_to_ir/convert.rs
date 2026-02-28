// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc::Unit;

/// Convert a hhbc::Unit to an ir::Unit.
///
/// Most of the outer structure of the hhbc::Unit maps 1:1 with ir::Unit. As a
/// result the "interesting" work is in the conversion of the bytecode to IR
/// when converting functions and methods (see `convert_body` in func.rs).
pub fn bc_to_ir(unit: Unit) -> ir::Unit {
    ir::Unit {
        classes: (unit.classes.into_iter())
            .map(crate::class::convert_class)
            .collect(),
        constants: unit.constants,
        fatal: unit.fatal,
        file_attributes: unit.file_attributes,
        functions: (unit.functions.into_iter())
            .map(crate::func::convert_function)
            .collect(),
        module_use: unit.module_use,
        modules: unit.modules,
        symbol_refs: unit.symbol_refs,
        typedefs: unit.typedefs,
        error_symbols: unit.error_symbols,
        missing_symbols: unit.missing_symbols,
    }
}
