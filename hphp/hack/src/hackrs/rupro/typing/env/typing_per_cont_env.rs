// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::cell::RefCell;

use im::HashMap;
use ty::reason::Reason;
use utils::core::LocalId;

use crate::typing::env::typing_local_types::Local;
use crate::typing::env::typing_local_types::LocalMap;

/// All possible continuations.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum TypingContKey {
    Next,
    Continue,
    Break,
    Catch,
    Do,
    Exit,
    Fallthrough,
    Finally,
}

/// Mapping each continuation type to a locals environment.
#[derive(Debug)]
pub struct PerContEnv<R: Reason>(RefCell<HashMap<TypingContKey, PerContEntry<R>>>);

/// A locals environment for each continuation.
#[derive(Debug, Clone)]
pub struct PerContEntry<R: Reason> {
    local_types: LocalMap<R>,
}

impl<R: Reason> PerContEnv<R> {
    /// Create a new (empty) collection of environments for each continuation.
    ///
    /// Note that at the point of creation, the collection will be empty! You
    /// should call `initial_locals` to setup the `Next` continuation.
    pub fn new() -> Self {
        Self(RefCell::new(HashMap::new()))
    }

    /// Empty the `Next` continuation.
    pub fn initial_locals(&self) {
        self.0
            .borrow_mut()
            .insert(TypingContKey::Next, PerContEntry::new());
    }

    /// Add a local binding for the given continuation.
    pub fn add(&self, key: TypingContKey, x: LocalId, ty: Local<R>) {
        if let Some(cont) = self.0.borrow_mut().get_mut(&key) {
            cont.local_types.add(x, ty);
        }
    }

    /// Get a local binding for the given continuation.
    pub fn get(&self, key: TypingContKey, x: &LocalId) -> Option<Local<R>> {
        self.0
            .borrow()
            .get(&key)
            .and_then(|env| env.local_types.get(x))
            .cloned()
    }

    /// Is there a locals environment for the given continuation?
    pub fn has_cont(&self, key: TypingContKey) -> bool {
        self.0.borrow().get(&key).is_some()
    }
}

impl<R: Reason> PerContEntry<R> {
    /// Initialize a new empty environment that can be used for one
    /// continuation.
    fn new() -> Self {
        Self {
            local_types: LocalMap::new(),
        }
    }
}
