// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::Arc;

use ffi::Maybe;
use ffi::Slice;
use hhbc::Fatal;

use crate::adata::AdataCache;
use crate::strings::StringCache;

/// Convert an ir::Unit to a hhbc::Unit
///
/// Most of the outer structure of the hhbc::Unit maps 1:1 with ir::Unit. As a
/// result the "interesting" work is in the conversion of the IR to bytecode
/// when converting functions and methods (see `convert_func` in func.rs).
pub fn ir_to_bc<'a>(alloc: &'a bumpalo::Bump, ir_unit: ir::Unit<'a>) -> hhbc::Unit<'a> {
    let strings = StringCache::new(alloc, Arc::clone(&ir_unit.strings));

    let mut unit = UnitBuilder::new_in(alloc);

    for cls in ir_unit.classes.into_iter() {
        crate::class::convert_class(alloc, &mut unit, cls, &strings);
    }

    for function in ir_unit.functions.into_iter() {
        crate::func::convert_function(&mut unit, function, &strings);
    }

    let mut unit = unit.finish();

    unit.file_attributes = convert_attributes(ir_unit.file_attributes, &strings).into();
    unit.typedefs = ir_unit
        .typedefs
        .into_iter()
        .map(|td| crate::types::convert_typedef(td, &strings))
        .collect::<Vec<_>>()
        .into();
    unit.constants = ir_unit
        .constants
        .into_iter()
        .map(|c| crate::constant::convert_hack_constant(c, &strings))
        .collect::<Vec<_>>()
        .into();
    unit.modules = ir_unit
        .modules
        .into_iter()
        .map(|module| hhbc::Module {
            attributes: convert_attributes(module.attributes, &strings).into(),
            name: strings.lookup_class_name(module.name),
            span: module.src_loc.to_span(),
            doc_comment: module.doc_comment.into(),
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
            message: ffi::Str::new_slice(alloc, message),
        });
    }

    unit
}

pub(crate) struct UnitBuilder<'a> {
    pub adata_cache: AdataCache<'a>,
    pub functions: Vec<hhbc::Function<'a>>,
    pub classes: Vec<hhbc::Class<'a>>,
}

impl<'a> UnitBuilder<'a> {
    fn new_in(alloc: &'a bumpalo::Bump) -> Self {
        Self {
            adata_cache: AdataCache::new(alloc),
            classes: Default::default(),
            functions: Default::default(),
        }
    }

    fn finish(self) -> hhbc::Unit<'a> {
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

fn convert_symbol_refs<'a>(symbol_refs: &ir::unit::SymbolRefs<'a>) -> hhbc::SymbolRefs<'a> {
    hhbc::SymbolRefs {
        classes: symbol_refs.classes.clone().into(),
        constants: symbol_refs.constants.clone().into(),
        functions: symbol_refs.functions.clone().into(),
        includes: symbol_refs.includes.clone().into(),
    }
}

pub(crate) fn convert_attributes<'a>(
    attrs: Vec<ir::Attribute>,
    strings: &StringCache<'a>,
) -> Vec<hhbc::Attribute<'a>> {
    attrs
        .into_iter()
        .map(|attr| {
            let arguments = Vec::from_iter(
                attr.arguments
                    .into_iter()
                    .map(|arg| convert_typed_value(&arg, strings)),
            );
            hhbc::Attribute {
                name: strings.lookup_class_name(attr.name).as_ffi_str(),
                arguments: arguments.into(),
            }
        })
        .collect()
}

pub(crate) fn convert_typed_value<'a>(
    tv: &ir::TypedValue,
    strings: &StringCache<'a>,
) -> hhbc::TypedValue<'a> {
    match *tv {
        ir::TypedValue::Uninit => hhbc::TypedValue::Uninit,
        ir::TypedValue::Int(v) => hhbc::TypedValue::Int(v),
        ir::TypedValue::Bool(v) => hhbc::TypedValue::Bool(v),
        ir::TypedValue::Float(v) => hhbc::TypedValue::Float(v),
        ir::TypedValue::String(v) => hhbc::TypedValue::String(strings.lookup_ffi_str(v)),
        ir::TypedValue::LazyClass(v) => {
            hhbc::TypedValue::LazyClass(strings.lookup_class_name(v).as_ffi_str())
        }
        ir::TypedValue::Null => hhbc::TypedValue::Null,
        ir::TypedValue::Vec(ref vs) => hhbc::TypedValue::Vec(Slice::fill_iter(
            strings.alloc,
            vs.iter().map(|v| convert_typed_value(v, strings)),
        )),
        ir::TypedValue::Keyset(ref vs) => hhbc::TypedValue::Keyset(Slice::fill_iter(
            strings.alloc,
            vs.iter().map(|v| convert_array_key(v, strings)),
        )),
        ir::TypedValue::Dict(ref vs) => hhbc::TypedValue::Dict(Slice::fill_iter(
            strings.alloc,
            vs.iter().map(|(k, v)| {
                let key = convert_array_key(k, strings);
                let value = convert_typed_value(v, strings);
                hhbc::Entry { key, value }
            }),
        )),
    }
}

pub(crate) fn convert_array_key<'a>(
    tv: &ir::ArrayKey,
    strings: &StringCache<'a>,
) -> hhbc::TypedValue<'a> {
    match *tv {
        ir::ArrayKey::Int(v) => hhbc::TypedValue::Int(v),
        ir::ArrayKey::LazyClass(v) => {
            hhbc::TypedValue::LazyClass(strings.lookup_class_name(v).as_ffi_str())
        }
        ir::ArrayKey::String(v) => hhbc::TypedValue::String(strings.lookup_ffi_str(v)),
    }
}
