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
pub fn bc_to_ir(unit: &Unit) -> ir::Unit {
    let constants = unit.constants.clone().into();
    let file_attributes = unit.file_attributes.clone().into();
    let modules: Vec<ir::Module> = unit.modules.clone().into();
    let typedefs: Vec<_> = unit.typedefs.clone().into();

    let mut ir_unit = ir::Unit {
        classes: Default::default(),
        constants,
        fatal: Default::default(),
        file_attributes,
        functions: Default::default(),
        module_use: unit.module_use.into(),
        modules,
        symbol_refs: unit.symbol_refs.clone(),
        typedefs,
    };

    // Convert the class containers - but not the methods which are converted
    // below.  This has to be done before the functions.
    for c in unit.classes.as_ref() {
        crate::class::convert_class(&mut ir_unit, c);
    }

    for f in unit.functions.as_ref() {
        crate::func::convert_function(&mut ir_unit, f, &unit.adata);
    }

    // This is where we convert the methods for all the classes.
    for (idx, c) in unit.classes.as_ref().iter().enumerate() {
        for m in c.methods.as_ref() {
            crate::func::convert_method(&mut ir_unit, idx, m, &unit.adata);
        }
    }

    ir_unit.fatal = unit.fatal.clone().into();

    ir_unit
}
