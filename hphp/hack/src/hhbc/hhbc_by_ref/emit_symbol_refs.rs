// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use decl_provider::DeclProvider;
use hhbc_by_ref_env::emitter::Emitter;
use hhbc_by_ref_hhas_symbol_refs::*;
use hhbc_by_ref_hhbc_id::{class, r#const, function, Id};
use hhbc_by_ref_symbol_refs_state::SymbolRefsState;

use std::collections::BTreeSet;

pub fn add_include<'arena, 'decl, D: DeclProvider<'decl>>(
    alloc: &'arena bumpalo::Bump,
    e: &mut Emitter<'arena, 'decl, D>,
    inc: IncludePath,
) {
    e.emit_symbol_refs_state_mut(alloc)
        .symbol_refs
        .includes
        .insert(inc);
}

pub fn add_constant<'arena, 'decl, D: DeclProvider<'decl>>(
    alloc: &'arena bumpalo::Bump,
    e: &mut Emitter<'arena, 'decl, D>,
    s: r#const::Type,
) {
    if !s.to_raw_string().is_empty() {
        e.emit_symbol_refs_state_mut(alloc)
            .symbol_refs
            .constants
            .insert(s.into());
    }
}

pub fn add_class<'arena, 'decl, D: DeclProvider<'decl>>(
    alloc: &'arena bumpalo::Bump,
    e: &mut Emitter<'arena, 'decl, D>,
    s: class::Type,
) {
    if !s.to_raw_string().is_empty() {
        e.emit_symbol_refs_state_mut(alloc)
            .symbol_refs
            .classes
            .insert(s.into());
    }
}

pub fn reset<'arena, 'decl, D: DeclProvider<'decl>>(
    alloc: &'arena bumpalo::Bump,
    e: &mut Emitter<'arena, 'decl, D>,
) {
    e.emit_symbol_refs_state_mut(alloc).symbol_refs = HhasSymbolRefs {
        includes: BTreeSet::new(),
        constants: BTreeSet::new(),
        functions: BTreeSet::new(),
        classes: BTreeSet::new(),
    };
}

pub fn add_function<'arena, 'decl, D: DeclProvider<'decl>>(
    alloc: &'arena bumpalo::Bump,
    e: &mut Emitter<'arena, 'decl, D>,
    s: function::Type,
) {
    if !s.to_raw_string().is_empty() {
        e.emit_symbol_refs_state_mut(alloc)
            .symbol_refs
            .functions
            .insert(s.into());
    }
}

pub fn take<'arena, 'decl, D: DeclProvider<'decl>>(
    alloc: &'arena bumpalo::Bump,
    e: &mut Emitter<'arena, 'decl, D>,
) -> SymbolRefsState {
    let state = e.emit_symbol_refs_state_mut(alloc);
    std::mem::take(state) // Replace `state` with the default
    // `SymbolRefsState` value and return the previous `state` value.
}
