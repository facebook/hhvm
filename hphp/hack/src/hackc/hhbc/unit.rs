// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use ffi::Vector;
use serde::Serialize;

use crate::Adata;
use crate::Attribute;
use crate::Class;
use crate::Constant;
use crate::FatalOp;
use crate::Function;
use crate::Module;
use crate::ModuleName;
use crate::SrcLoc;
use crate::StringId;
use crate::SymbolRefs;
use crate::Typedef;

#[derive(Debug, Serialize)]
#[repr(C)]
pub struct Unit {
    pub adata: Vector<Adata>,
    pub functions: Vector<Function>,
    pub classes: Vector<Class>,
    pub modules: Vector<Module>,
    pub typedefs: Vector<Typedef>,
    pub file_attributes: Vector<Attribute>,
    pub module_use: Maybe<ModuleName>,
    pub symbol_refs: SymbolRefs,
    pub constants: Vector<Constant>,

    /// If the Unit failed to parse or compile this defines the error that
    /// should be reported and the rest of the Unit will be empty.
    pub fatal: Maybe<Fatal>,

    pub missing_symbols: Vector<StringId>,
    pub error_symbols: Vector<StringId>,
    // TODO(T120858428): Remove this field once non-utf8 is banned from the
    // parser.
    pub valid_utf8: bool,
    pub invalid_utf8_offset: usize,
}

/// Fields used when a unit had compile-time errors that should be reported
/// when the unit is loaded.
#[derive(Debug, Clone, Serialize)]
#[repr(C)]
pub struct Fatal {
    pub op: FatalOp,
    pub loc: SrcLoc,
    pub message: Vector<u8>,
}
