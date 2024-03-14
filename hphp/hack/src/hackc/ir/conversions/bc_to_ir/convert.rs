// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::Arc;

use hash::HashMap;
use hhbc::Unit;

/// Convert a hhbc::Unit to an ir::Unit.
///
/// Most of the outer structure of the hhbc::Unit maps 1:1 with ir::Unit. As a
/// result the "interesting" work is in the conversion of the bytecode to IR
/// when converting functions and methods (see `convert_body` in func.rs).
pub fn bc_to_ir(unit: &Unit) -> ir::Unit {
    // Traditionally the HHBC AdataIds are named A_# - but let's not rely on
    // that.
    let adata_lookup = unit
        .adata
        .iter()
        .map(|hhbc::Adata { id, value }| (*id, Arc::new(value.clone())))
        .collect();

    let unit_state = UnitState { adata_lookup };

    let constants = unit
        .constants
        .as_ref()
        .iter()
        .map(crate::constant::convert_constant)
        .collect();

    let file_attributes: Vec<_> = unit.file_attributes.iter().map(convert_attribute).collect();

    let modules: Vec<ir::Module> = unit
        .modules
        .iter()
        .map(|module| ir::Module {
            attributes: module.attributes.iter().map(convert_attribute).collect(),
            name: module.name,
            src_loc: ir::SrcLoc::from_span(&module.span),
            doc_comment: module.doc_comment.clone().map(|c| c.into()).into(),
        })
        .collect();

    let typedefs: Vec<_> = unit
        .typedefs
        .iter()
        .map(crate::types::convert_typedef)
        .collect();

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
        crate::func::convert_function(&mut ir_unit, f, &unit_state);
    }

    // This is where we convert the methods for all the classes.
    for (idx, c) in unit.classes.as_ref().iter().enumerate() {
        for m in c.methods.as_ref() {
            crate::func::convert_method(&mut ir_unit, idx, m, &unit_state);
        }
    }

    ir_unit.fatal = unit.fatal.clone().into();

    ir_unit
}

pub(crate) struct UnitState {
    /// Conversion from hhbc::AdataId to hhbc::TypedValue
    pub(crate) adata_lookup: HashMap<hhbc::AdataId, Arc<ir::TypedValue>>,
}

pub(crate) fn convert_attribute(attr: &hhbc::Attribute) -> ir::Attribute {
    let arguments = attr.arguments.clone().into();
    ir::Attribute {
        name: attr.name,
        arguments,
    }
}
