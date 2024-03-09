// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use hhbc::Fatal;

use crate::adata::AdataCache;

/// Convert an ir::Unit to a hhbc::Unit
///
/// Most of the outer structure of the hhbc::Unit maps 1:1 with ir::Unit. As a
/// result the "interesting" work is in the conversion of the IR to bytecode
/// when converting functions and methods (see `convert_func` in func.rs).
pub fn ir_to_bc(ir_unit: ir::Unit) -> hhbc::Unit {
    let mut unit = UnitBuilder::new();

    for cls in ir_unit.classes.into_iter() {
        crate::class::convert_class(&mut unit, cls);
    }

    for function in ir_unit.functions.into_iter() {
        crate::func::convert_function(&mut unit, function);
    }

    let mut unit = unit.finish();

    unit.file_attributes = convert_attributes(ir_unit.file_attributes).into();
    unit.typedefs = ir_unit
        .typedefs
        .into_iter()
        .map(crate::types::convert_typedef)
        .collect::<Vec<_>>()
        .into();
    unit.constants = ir_unit
        .constants
        .into_iter()
        .map(crate::constant::convert_hack_constant)
        .collect::<Vec<_>>()
        .into();
    unit.modules = ir_unit
        .modules
        .into_iter()
        .map(|module| hhbc::Module {
            attributes: convert_attributes(module.attributes).into(),
            name: module.name,
            span: module.src_loc.to_span(),
            doc_comment: module.doc_comment.map(|c| c.into()).into(),
            exports: Maybe::Nothing, // TODO
            imports: Maybe::Nothing, // TODO
        })
        .collect::<Vec<_>>()
        .into();
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
            loc: loc.to_hhbc(),
            message: message.to_vec().into(),
        });
    }

    unit
}

pub(crate) struct UnitBuilder {
    pub adata_cache: AdataCache,
    pub functions: Vec<hhbc::Function>,
    pub classes: Vec<hhbc::Class>,
}

impl UnitBuilder {
    fn new() -> Self {
        Self {
            adata_cache: AdataCache::new(),
            classes: Default::default(),
            functions: Default::default(),
        }
    }

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

pub(crate) fn convert_attributes(attrs: Vec<ir::Attribute>) -> Vec<hhbc::Attribute> {
    attrs
        .into_iter()
        .map(|attr| {
            let arguments = Vec::from_iter(
                attr.arguments
                    .into_iter()
                    .map(|arg| convert_typed_value(&arg)),
            );
            // XXX attribute names are class names
            hhbc::Attribute::new(attr.name.as_string_id(), arguments)
        })
        .collect()
}

pub(crate) fn convert_typed_value(tv: &ir::TypedValue) -> hhbc::TypedValue {
    match tv {
        ir::TypedValue::Uninit => hhbc::TypedValue::Uninit,
        ir::TypedValue::Int(v) => hhbc::TypedValue::Int(*v),
        ir::TypedValue::Bool(v) => hhbc::TypedValue::Bool(*v),
        ir::TypedValue::Float(v) => hhbc::TypedValue::Float(*v),
        ir::TypedValue::String(v) => hhbc::TypedValue::String(*v),
        ir::TypedValue::LazyClass(v) => hhbc::TypedValue::LazyClass(v.as_string_id()),
        ir::TypedValue::Null => hhbc::TypedValue::Null,
        ir::TypedValue::Vec(ref vs) => {
            hhbc::TypedValue::Vec(Vec::from_iter(vs.iter().map(convert_typed_value)).into())
        }
        ir::TypedValue::Keyset(ref vs) => {
            hhbc::TypedValue::Keyset(Vec::from_iter(vs.iter().map(convert_array_key)).into())
        }
        ir::TypedValue::Dict(ref vs) => hhbc::TypedValue::Dict(
            Vec::from_iter(vs.iter().map(|(k, v)| {
                let key = convert_array_key(k);
                let value = convert_typed_value(v);
                hhbc::Entry { key, value }
            }))
            .into(),
        ),
    }
}

pub(crate) fn convert_array_key(tv: &ir::ArrayKey) -> hhbc::TypedValue {
    match *tv {
        ir::ArrayKey::Int(v) => hhbc::TypedValue::Int(v),
        ir::ArrayKey::LazyClass(v) => hhbc::TypedValue::LazyClass(v.as_string_id()),
        ir::ArrayKey::String(v) => hhbc::TypedValue::String(v),
    }
}
