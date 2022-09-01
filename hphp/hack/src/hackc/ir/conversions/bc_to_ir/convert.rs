// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use hash::HashMap;
use hhbc::Unit;

/// Convert a hhbc::Unit to an ir::Unit.
///
/// Most of the outer structure of the hhbc::Unit maps 1:1 with ir::Unit. As a
/// result the "interesting" work is in the conversion of the bytecode to IR
/// when converting functions and methods (see `convert_body` in func.rs).
///
/// NOTE: hhbc::Unit has to be by-ref because it unfortunately contains a bunch
/// of ffi::Slice<T> which cannot own T.
///
pub fn bc_to_ir<'a>(unit: &'_ Unit<'a>) -> ir::Unit<'a> {
    let mut strings = ir::StringInterner::default();

    let adata: HashMap<_, _> = unit.adata.iter().map(|a| (a.id, a.value.clone())).collect();

    let constants = unit
        .constants
        .as_ref()
        .iter()
        .map(crate::constant::convert_constant)
        .collect();

    let file_attributes: Vec<_> = unit.file_attributes.iter().map(convert_attribute).collect();

    let modules: Vec<ir::Module<'a>> = unit
        .modules
        .iter()
        .map(|module| ir::Module {
            attributes: module.attributes.iter().map(convert_attribute).collect(),
            name: ir::ClassId::from_hhbc(module.name, &mut strings),
            span: module.span,
        })
        .collect();

    let symbol_refs = convert_symbol_refs(&unit.symbol_refs);

    let typedefs: Vec<_> = unit.typedefs.iter().cloned().collect();

    let mut ir_unit = ir::Unit {
        adata,
        classes: Default::default(),
        constants,
        fatal: Default::default(),
        file_attributes,
        functions: Default::default(),
        module_use: unit.module_use.into(),
        modules,
        strings,
        symbol_refs,
        typedefs,
    };

    // Convert the class containers - but not the methods which are converted
    // below.  This has to be done before the functions.
    for c in unit.classes.as_ref() {
        crate::class::convert_class(&mut ir_unit, c);
    }

    for f in unit.functions.as_ref() {
        crate::func::convert_function(&mut ir_unit, f);
    }

    // This is where we convert the methods for all the classes.
    for (idx, c) in unit.classes.as_ref().iter().enumerate() {
        for m in c.methods.as_ref() {
            crate::func::convert_method(&mut ir_unit, idx, m);
        }
    }

    if let Maybe::Just(ffi::Triple(op, loc, msg)) = unit.fatal {
        ir_unit.fatal = match op {
            hhbc::FatalOp::Parse => ir::FatalOp::Parse(loc, msg),
            hhbc::FatalOp::Runtime => ir::FatalOp::Runtime(loc, msg),
            hhbc::FatalOp::RuntimeOmitFrame => ir::FatalOp::RuntimeOmitFrame(loc, msg),
            _ => panic!("bad FatalOp value"),
        };
    }

    ir_unit
}

pub(crate) fn convert_attribute<'a>(attr: &hhbc::Attribute<'a>) -> ir::Attribute<'a> {
    ir::Attribute {
        name: attr.name,
        arguments: attr.arguments.as_ref().to_vec(),
    }
}

fn convert_symbol_refs<'a>(symbol_refs: &hhbc::SymbolRefs<'a>) -> ir::unit::SymbolRefs<'a> {
    // TODO: It would be nice if we could determine this stuff from the IR
    // instead of having to carry it along with the Unit.

    let classes = symbol_refs.classes.iter().cloned().collect();
    let constants = symbol_refs.constants.iter().cloned().collect();
    let functions = symbol_refs.functions.iter().cloned().collect();
    let includes = symbol_refs.includes.iter().cloned().collect();

    ir::unit::SymbolRefs {
        classes,
        constants,
        functions,
        includes,
    }
}
