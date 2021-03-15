// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_by_ref_env::emitter::Emitter;
use hhbc_by_ref_hhas_symbol_refs::*;
use hhbc_by_ref_hhbc_id::{class, r#const, function, Id};
use hhbc_by_ref_symbol_refs_state::SymbolRefsState;
use std::collections::BTreeSet;

pub fn get_symbol_refs<'a, 'arena: 'a>(
    alloc: &'arena bumpalo::Bump,
    e: &'a mut Emitter<'arena>,
) -> &'a HhasSymbolRefs {
    &e.emit_symbol_refs_state_mut(alloc).symbol_refs
}

pub fn set_symbol_refs<'arena>(
    alloc: &'arena bumpalo::Bump,
    e: &mut Emitter<'arena>,
    s: HhasSymbolRefs,
) {
    e.emit_symbol_refs_state_mut(alloc).symbol_refs = s;
}

pub fn add_include<'arena>(
    alloc: &'arena bumpalo::Bump,
    e: &mut Emitter<'arena>,
    inc: IncludePath,
) {
    e.emit_symbol_refs_state_mut(alloc)
        .symbol_refs
        .includes
        .insert(inc);
}

pub fn add_constant<'arena>(
    alloc: &'arena bumpalo::Bump,
    e: &mut Emitter<'arena>,
    s: r#const::Type,
) {
    if !s.to_raw_string().is_empty() {
        e.emit_symbol_refs_state_mut(alloc)
            .symbol_refs
            .constants
            .insert(s.into());
    }
}

pub fn add_class<'arena>(alloc: &'arena bumpalo::Bump, e: &mut Emitter<'arena>, s: class::Type) {
    if !s.to_raw_string().is_empty() {
        e.emit_symbol_refs_state_mut(alloc)
            .symbol_refs
            .classes
            .insert(s.into());
    }
}

pub fn reset<'arena>(alloc: &'arena bumpalo::Bump, e: &mut Emitter<'arena>) {
    e.emit_symbol_refs_state_mut(alloc).symbol_refs = HhasSymbolRefs {
        includes: BTreeSet::new(),
        constants: BTreeSet::new(),
        functions: BTreeSet::new(),
        classes: BTreeSet::new(),
    };
}

pub fn add_function<'arena>(
    alloc: &'arena bumpalo::Bump,
    e: &mut Emitter<'arena>,
    s: function::Type,
) {
    if !s.to_raw_string().is_empty() {
        e.emit_symbol_refs_state_mut(alloc)
            .symbol_refs
            .functions
            .insert(s.into());
    }
}

pub fn take<'arena>(alloc: &'arena bumpalo::Bump, e: &mut Emitter<'arena>) -> SymbolRefsState {
    let state = e.emit_symbol_refs_state_mut(alloc);
    std::mem::take(state) // Replace `state` with the default
    // `SymbolRefsState` value and return the previous `state` value.
}
