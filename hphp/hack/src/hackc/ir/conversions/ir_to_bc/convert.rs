// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

/// Convert an ir::Unit to a hhbc::Unit
///
/// Most of the outer structure of the hhbc::Unit maps 1:1 with ir::Unit. As a
/// result the "interesting" work is in the conversion of the IR to bytecode
/// when converting functions and methods (see `convert_func` in func.rs).
pub fn ir_to_bc(ir_unit: ir::Unit) -> hhbc::Unit {
    let mut unit = UnitBuilder::default();

    for cls in ir_unit.classes.into_iter() {
        crate::class::convert_class(&mut unit, cls);
    }

    for function in ir_unit.functions.into_iter() {
        unit.functions.push(crate::func::convert_function(
            function,
            &mut unit.adata_cache,
        ));
    }

    hhbc::Unit {
        functions: unit.functions.into(),
        classes: unit.classes.into(),
        file_attributes: ir_unit.file_attributes,
        typedefs: ir_unit.typedefs,
        constants: ir_unit.constants,
        modules: ir_unit.modules,
        module_use: ir_unit.module_use,
        symbol_refs: ir_unit.symbol_refs,
        fatal: ir_unit.fatal,
        missing_symbols: ir_unit.missing_symbols,
        error_symbols: ir_unit.error_symbols,
    }
}

#[derive(Default)]
pub(crate) struct UnitBuilder {
    pub adata_cache: hhbc::AdataState,
    pub functions: Vec<hhbc::Function>,
    pub classes: Vec<hhbc::Class>,
}
