// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use ffi::Str;
use ffi::Vector;
use serde::Serialize;

use crate::Adata;
use crate::Attribute;
use crate::Class;
use crate::Constant;
use crate::FatalOp;
use crate::Function;
use crate::Module;
use crate::SrcLoc;
use crate::SymbolRefs;
use crate::Typedef;

#[derive(Debug, Serialize)]
#[repr(C)]
pub struct Unit<'arena> {
    pub adata: Vector<Adata<'arena>>,
    pub functions: Vector<Function<'arena>>,
    pub classes: Vector<Class<'arena>>,
    pub modules: Vector<Module<'arena>>,
    pub typedefs: Vector<Typedef<'arena>>,
    pub file_attributes: Vector<Attribute<'arena>>,
    pub module_use: Maybe<Str<'arena>>,
    pub symbol_refs: SymbolRefs<'arena>,
    pub constants: Vector<Constant<'arena>>,
    pub fatal: Maybe<Fatal<'arena>>,
    pub missing_symbols: Vector<Str<'arena>>,
    pub error_symbols: Vector<Str<'arena>>,
    // TODO(T120858428): Remove this field once non-utf8 is banned from the
    // parser.
    pub valid_utf8: bool,
    pub invalid_utf8_offset: usize,
}

/// Fields used when a unit had compile-time errors that should be reported
/// when the unit is loaded.
#[derive(Debug, Serialize)]
#[repr(C)]
pub struct Fatal<'arena> {
    pub op: FatalOp,
    pub loc: SrcLoc,
    pub message: Str<'arena>,
}
