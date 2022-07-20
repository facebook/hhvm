// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use ffi::Slice;
use ffi::Str;
use itertools::Itertools;

/// Convert an ir::Unit to a HackCUnit
///
/// Most of the outer structure of the HackCUnit maps 1:1 with ir::Unit. As a
/// result the "interesting" work is in the conversion of the IR to bytecode
/// when converting functions and methods (see `convert_func` in func.rs).
///
pub fn ir_to_bc<'a>(
    alloc: &'a bumpalo::Bump,
    ir_unit: ir::Unit<'a>,
) -> hhbc::hackc_unit::HackCUnit<'a> {
    let class_order = ir_unit.classes.iter().map(|cls| cls.name).collect_vec();

    let mut unit = HackCUnitBuilder::new_in(alloc, class_order);

    unit.adata = Slice::fill_iter(
        alloc,
        ir_unit
            .adata
            .into_iter()
            .map(|(name, value)| convert_adata(alloc, name, value)),
    );

    for cls in ir_unit.classes.into_iter() {
        crate::class::convert_class(alloc, &mut unit, cls, &ir_unit.strings);
    }

    for function in ir_unit.functions.into_iter() {
        crate::func::convert_function(alloc, &mut unit, function, &ir_unit.strings);
    }

    let mut unit = unit.finish();

    unit.file_attributes = convert_attributes(alloc, ir_unit.file_attributes);
    unit.typedefs = Slice::fill_iter(alloc, ir_unit.typedefs.into_iter());
    unit.constants = Slice::fill_iter(
        alloc,
        ir_unit
            .constants
            .into_iter()
            .map(crate::constant::convert_hack_constant),
    );
    unit.modules = Slice::fill_iter(
        alloc,
        ir_unit
            .modules
            .into_iter()
            .map(|module| hhbc::hhas_module::HhasModule {
                attributes: convert_attributes(alloc, module.attributes),
                name: module.name.to_hhbc(alloc, &ir_unit.strings),
                span: module.span,
            }),
    );
    unit.module_use = ir_unit.module_use.into();
    unit.symbol_refs = convert_symbol_refs(alloc, &ir_unit.symbol_refs);

    match &ir_unit.fatal {
        ir::FatalOp::None => {}
        ir::FatalOp::Parse(loc, msg)
        | ir::FatalOp::Runtime(loc, msg)
        | ir::FatalOp::RuntimeOmitFrame(loc, msg) => {
            let op = match ir_unit.fatal {
                ir::FatalOp::None => unreachable!(),
                ir::FatalOp::Parse(..) => hhbc::FatalOp::Parse,
                ir::FatalOp::Runtime(..) => hhbc::FatalOp::Runtime,
                ir::FatalOp::RuntimeOmitFrame(..) => hhbc::FatalOp::RuntimeOmitFrame,
            };
            let pos = hhbc::hhas_pos::HhasPos {
                line_begin: loc.line_begin as usize,
                col_begin: loc.col_begin as usize,
                line_end: loc.line_end as usize,
                col_end: loc.col_end as usize,
            };
            unit.fatal = Maybe::Just(ffi::Triple(op, pos, *msg))
        }
    }

    unit
}

pub(crate) struct HackCUnitBuilder<'a> {
    pub adata: Slice<'a, hhbc::hhas_adata::HhasAdata<'a>>,
    pub functions: bumpalo::collections::Vec<'a, hhbc::hhas_function::HhasFunction<'a>>,
    pub classes: bumpalo::collections::Vec<'a, hhbc::hhas_class::HhasClass<'a>>,
    pub class_order: Vec<ir::ClassId>,
}

impl<'a> HackCUnitBuilder<'a> {
    fn new_in(alloc: &'a bumpalo::Bump, class_order: Vec<ir::ClassId>) -> Self {
        Self {
            adata: Slice::empty(),
            classes: bumpalo::collections::Vec::new_in(alloc),
            functions: bumpalo::collections::Vec::new_in(alloc),
            class_order,
        }
    }

    fn finish(self) -> hhbc::hackc_unit::HackCUnit<'a> {
        hhbc::hackc_unit::HackCUnit {
            adata: self.adata,
            functions: self.functions.into_bump_slice().into(),
            classes: self.classes.into_bump_slice().into(),
            typedefs: Default::default(),
            file_attributes: Default::default(),
            modules: Default::default(),
            module_use: Maybe::Nothing,
            symbol_refs: Default::default(),
            constants: Default::default(),
            fatal: Default::default(),
        }
    }
}

fn convert_adata<'a>(
    _alloc: &'a bumpalo::Bump,
    name: Str<'a>,
    value: ir::TypedValue<'a>,
) -> hhbc::hhas_adata::HhasAdata<'a> {
    hhbc::hhas_adata::HhasAdata { id: name, value }
}

fn convert_symbol_refs<'a>(
    alloc: &'a bumpalo::Bump,
    symbol_refs: &ir::unit::SymbolRefs<'a>,
) -> hhbc::hhas_symbol_refs::HhasSymbolRefs<'a> {
    let classes = Slice::fill_iter(alloc, symbol_refs.classes.iter().cloned());
    let constants = Slice::fill_iter(alloc, symbol_refs.constants.iter().cloned());
    let functions = Slice::fill_iter(alloc, symbol_refs.functions.iter().cloned());
    let includes = Slice::fill_iter(alloc, symbol_refs.includes.iter().cloned());

    hhbc::hhas_symbol_refs::HhasSymbolRefs {
        classes,
        constants,
        functions,
        includes,
    }
}

pub(crate) fn convert_attributes<'a>(
    alloc: &'a bumpalo::Bump,
    attrs: Vec<ir::Attribute<'a>>,
) -> Slice<'a, hhbc::hhas_attribute::HhasAttribute<'a>> {
    Slice::fill_iter(
        alloc,
        attrs
            .into_iter()
            .map(|attr| hhbc::hhas_attribute::HhasAttribute {
                name: attr.name,
                arguments: Slice::fill_iter(alloc, attr.arguments.into_iter()),
            }),
    )
}
