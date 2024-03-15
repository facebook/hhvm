// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use hhbc::Fatal;

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
        crate::func::convert_function(&mut unit, function);
    }

    let mut unit = unit.finish();

    unit.file_attributes = ir_unit.file_attributes.into();
    unit.typedefs = ir_unit
        .typedefs
        .into_iter()
        .map(crate::types::convert_typedef)
        .collect::<Vec<_>>()
        .into();
    unit.constants = ir_unit.constants.into();
    unit.modules = ir_unit.modules.into();
    unit.module_use = ir_unit.module_use.into();
    unit.symbol_refs = convert_symbol_refs(&ir_unit.symbol_refs);

    if let Some(ir::Fatal { op, loc, message }) = ir_unit.fatal.as_ref() {
        let op = match *op {
            ir::FatalOp::Parse => hhbc::FatalOp::Parse,
            ir::FatalOp::Runtime => hhbc::FatalOp::Runtime,
            ir::FatalOp::RuntimeOmitFrame => hhbc::FatalOp::RuntimeOmitFrame,
            _ => unreachable!(),
        };
        unit.fatal = Maybe::Just(Fatal {
            op,
            loc: *loc,
            message: message.to_vec().into(),
        });
    }

    unit
}

#[derive(Default)]
pub(crate) struct UnitBuilder {
    pub adata_cache: hhbc::AdataState,
    pub functions: Vec<hhbc::Function>,
    pub classes: Vec<hhbc::Class>,
}

impl UnitBuilder {
    fn finish(self) -> hhbc::Unit {
        hhbc::Unit {
            adata: self.adata_cache.finish().into(),
            functions: self.functions.into(),
            classes: self.classes.into(),
            typedefs: Default::default(),
            file_attributes: Default::default(),
            modules: Default::default(),
            module_use: Maybe::Nothing,
            symbol_refs: Default::default(),
            constants: Default::default(),
            fatal: Default::default(),
            missing_symbols: Default::default(),
            error_symbols: Default::default(),
            valid_utf8: true,
            invalid_utf8_offset: 0,
        }
    }
}

fn convert_symbol_refs(symbol_refs: &ir::SymbolRefs) -> hhbc::SymbolRefs {
    hhbc::SymbolRefs {
        classes: symbol_refs.classes.clone(),
        constants: symbol_refs.constants.clone(),
        functions: symbol_refs.functions.clone(),
        includes: symbol_refs.includes.clone(),
    }
}
