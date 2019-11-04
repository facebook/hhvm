// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use env::emitter::Emitter;
use hhas_symbol_refs_rust::*;
use std::collections::BTreeSet;

struct State {
    symbol_refs: HhasSymbolRefs,
}

impl State {
    fn init() -> Box<dyn std::any::Any> {
        Box::new(State {
            symbol_refs: HhasSymbolRefs {
                includes: BTreeSet::new(),
                constants: BTreeSet::new(),
                functions: BTreeSet::new(),
                classes: BTreeSet::new(),
            },
        })
    }

    pub fn get_symbol_refs(e: &mut Emitter) -> &HhasSymbolRefs {
        &e.emit_state_mut().symbol_refs
    }

    pub fn set_symbol_refs(e: &mut Emitter, s: HhasSymbolRefs) {
        e.emit_state_mut().symbol_refs = s;
    }

    pub fn add_include(e: &mut Emitter, inc: IncludePath) {
        e.emit_state_mut().symbol_refs.includes.insert(inc);
    }

    pub fn add_constant(e: &mut Emitter, s: String) {
        if !s.is_empty() {
            e.emit_state_mut().symbol_refs.constants.insert(s);
        }
    }

    pub fn add_function(e: &mut Emitter, s: String) {
        if !s.is_empty() {
            e.emit_state_mut().symbol_refs.functions.insert(s);
        }
    }

    pub fn add_class(e: &mut Emitter, s: String) {
        if !s.is_empty() {
            e.emit_state_mut().symbol_refs.classes.insert(s);
        }
    }

    pub fn reset(e: &mut Emitter) {
        e.emit_state_mut().symbol_refs = HhasSymbolRefs {
            includes: BTreeSet::new(),
            constants: BTreeSet::new(),
            functions: BTreeSet::new(),
            classes: BTreeSet::new(),
        };
    }
}

env::lazy_emit_state!(symbol_refs_state, State, State::init);
