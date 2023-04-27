// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use ffi::Slice;
use ffi::Str;
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

#[derive(Default, Debug, Serialize)]
#[repr(C)]
pub struct Unit<'arena> {
    pub adata: Slice<'arena, Adata<'arena>>,
    pub functions: Slice<'arena, Function<'arena>>,
    pub classes: Slice<'arena, Class<'arena>>,
    pub modules: Slice<'arena, Module<'arena>>,
    pub typedefs: Slice<'arena, Typedef<'arena>>,
    pub file_attributes: Slice<'arena, Attribute<'arena>>,
    pub module_use: Maybe<Str<'arena>>,
    pub symbol_refs: SymbolRefs<'arena>,
    pub constants: Slice<'arena, Constant<'arena>>,
    pub fatal: Maybe<Fatal<'arena>>,
    pub missing_symbols: Slice<'arena, Str<'arena>>,
    pub error_symbols: Slice<'arena, Str<'arena>>,
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
