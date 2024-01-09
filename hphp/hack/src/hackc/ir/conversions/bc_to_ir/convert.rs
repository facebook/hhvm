// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::Path;
use std::sync::Arc;

use ffi::Maybe;
use hash::HashMap;
use hhbc::Fatal;
use hhbc::Unit;
use ir::StringInterner;

/// Convert a hhbc::Unit to an ir::Unit.
///
/// Most of the outer structure of the hhbc::Unit maps 1:1 with ir::Unit. As a
/// result the "interesting" work is in the conversion of the bytecode to IR
/// when converting functions and methods (see `convert_body` in func.rs).
///
/// NOTE: hhbc::Unit has to be by-ref because it unfortunately contains a bunch
/// of ffi::Slice<T> which cannot own T.
pub fn bc_to_ir<'a>(unit: &'_ Unit<'a>, filename: &Path) -> ir::Unit<'a> {
    use std::os::unix::ffi::OsStrExt;
    let strings = Arc::new(ir::StringInterner::default());

    let filename = ir::Filename(strings.intern_bytes(filename.as_os_str().as_bytes()));

    // Traditionally the HHBC AdataIds are named A_# - but let's not rely on
    // that.
    let adata_lookup = unit
        .adata
        .iter()
        .map(|hhbc::Adata { id, value }| (*id, Arc::new(convert_typed_value(value, &strings))))
        .collect();

    let unit_state = UnitState { adata_lookup };

    let constants = unit
        .constants
        .as_ref()
        .iter()
        .map(|c| crate::constant::convert_constant(c, &strings))
        .collect();

    let file_attributes: Vec<_> = unit
        .file_attributes
        .iter()
        .map(|a| convert_attribute(a, &strings))
        .collect();

    let modules: Vec<ir::Module<'a>> = unit
        .modules
        .iter()
        .map(|module| ir::Module {
            attributes: module
                .attributes
                .iter()
                .map(|a| convert_attribute(a, &strings))
                .collect(),
            name: ir::ClassId::from_hhbc(module.name, &strings),
            src_loc: ir::SrcLoc::from_span(filename, &module.span),
            doc_comment: module.doc_comment.into(),
        })
        .collect();

    let symbol_refs = convert_symbol_refs(&unit.symbol_refs);

    let typedefs: Vec<_> = unit
        .typedefs
        .iter()
        .map(|td| crate::types::convert_typedef(td, filename, &strings))
        .collect();

    let mut ir_unit = ir::Unit {
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
        crate::class::convert_class(&mut ir_unit, filename, c);
    }

    for f in unit.functions.as_ref() {
        crate::func::convert_function(&mut ir_unit, filename, f, &unit_state);
    }

    // This is where we convert the methods for all the classes.
    for (idx, c) in unit.classes.as_ref().iter().enumerate() {
        for m in c.methods.as_ref() {
            crate::func::convert_method(&mut ir_unit, filename, idx, m, &unit_state);
        }
    }

    if let Maybe::Just(Fatal { op, loc, message }) = unit.fatal {
        let loc = ir::func::SrcLoc::from_hhbc(filename, &loc);
        let message = bstr::BString::from(message.as_ref());
        ir_unit.fatal = Some(ir::Fatal { op, loc, message });
    }

    ir_unit
}

pub(crate) struct UnitState<'a> {
    /// Conversion from hhbc::AdataId to hhbc::TypedValue
    pub(crate) adata_lookup: HashMap<hhbc::AdataId<'a>, Arc<ir::TypedValue>>,
}

pub(crate) fn convert_attribute<'a>(
    attr: &hhbc::Attribute<'a>,
    strings: &StringInterner,
) -> ir::Attribute {
    let arguments = attr
        .arguments
        .iter()
        .map(|tv| convert_typed_value(tv, strings))
        .collect();
    ir::Attribute {
        name: ir::ClassId::from_hhbc(hhbc::ClassName::new(attr.name), strings),
        arguments,
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

pub(crate) fn convert_typed_value<'a>(
    tv: &hhbc::TypedValue<'a>,
    strings: &StringInterner,
) -> ir::TypedValue {
    match *tv {
        hhbc::TypedValue::Uninit => ir::TypedValue::Uninit,
        hhbc::TypedValue::Int(v) => ir::TypedValue::Int(v),
        hhbc::TypedValue::Bool(v) => ir::TypedValue::Bool(v),
        hhbc::TypedValue::Float(v) => ir::TypedValue::Float(v),
        hhbc::TypedValue::String(v) => ir::TypedValue::String(strings.intern_bytes(v.as_ref())),
        hhbc::TypedValue::LazyClass(v) => {
            ir::TypedValue::LazyClass(ir::ClassId::from_hhbc(hhbc::ClassName::new(v), strings))
        }
        hhbc::TypedValue::Null => ir::TypedValue::Null,
        hhbc::TypedValue::Vec(vs) => ir::TypedValue::Vec(
            vs.iter()
                .map(|tv| convert_typed_value(tv, strings))
                .collect(),
        ),
        hhbc::TypedValue::Keyset(vs) => {
            ir::TypedValue::Keyset(vs.iter().map(|tv| convert_array_key(tv, strings)).collect())
        }
        hhbc::TypedValue::Dict(vs) => ir::TypedValue::Dict(
            vs.iter()
                .map(|hhbc::Entry { key, value }| {
                    let key = convert_array_key(key, strings);
                    let value = convert_typed_value(value, strings);
                    (key, value)
                })
                .collect(),
        ),
    }
}

pub(crate) fn convert_array_key<'a>(
    tv: &hhbc::TypedValue<'a>,
    strings: &StringInterner,
) -> ir::ArrayKey {
    match *tv {
        hhbc::TypedValue::Int(v) => ir::ArrayKey::Int(v),
        hhbc::TypedValue::LazyClass(v) => {
            ir::ArrayKey::LazyClass(ir::ClassId::from_hhbc(hhbc::ClassName::new(v), strings))
        }
        hhbc::TypedValue::String(v) => ir::ArrayKey::String(strings.intern_bytes(v.as_ref())),
        _ => panic!("Unable to convert {tv:?} to ArrayKey"),
    }
}
