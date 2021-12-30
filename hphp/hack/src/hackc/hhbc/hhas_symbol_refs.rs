// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::{Slice, Str};
pub use symbol_refs_state::IncludePath;
use symbol_refs_state::SymbolRefsState;

/// Data structure for keeping track of symbols (and includes) we
/// encounter in the course of emitting bytecode for an AST. We split
/// them into these four categories for the sake of HHVM, which has
/// a dedicated lookup function corresponding to each.
#[derive(Default, Clone, Debug)]
#[repr(C)]
pub struct HhasSymbolRefs<'arena> {
    pub includes: Slice<'arena, IncludePath<'arena>>,
    pub constants: Slice<'arena, Str<'arena>>,
    pub functions: Slice<'arena, Str<'arena>>,
    pub classes: Slice<'arena, Str<'arena>>,
}

impl<'arena> HhasSymbolRefs<'arena> {
    /// It's not possible to provide an instance for
    /// `std::convert::From` due to the need for an allocator so
    /// provide this associated function instead.
    pub fn from_symbol_refs_state(
        alloc: &'arena bumpalo::Bump,
        symbols: SymbolRefsState<'arena>,
    ) -> Self {
        HhasSymbolRefs {
            includes: Slice::new(alloc.alloc_slice_fill_iter(symbols.includes.into_iter())),
            constants: Slice::new(alloc.alloc_slice_fill_iter(symbols.constants.into_iter())),
            functions: Slice::new(alloc.alloc_slice_fill_iter(symbols.functions.into_iter())),
            classes: Slice::new(alloc.alloc_slice_fill_iter(symbols.classes.into_iter())),
        }
    }
}
