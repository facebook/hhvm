// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod changes_store;

use std::fmt::Debug;

pub use changes_store::ChangesStore;

/// A threadsafe datastore, intended for global decl storage. The key type is
/// intended to be a `Symbol` or tuple of `Symbol`s, and the value type is
/// intended to be a ref-counted pointer (like `Arc` or `Hc`).
pub trait Store<K: Copy, V>: Debug + Send + Sync {
    fn get(&self, key: K) -> Option<V>;
    fn insert(&self, key: K, val: V);
}

/// A thread-local datastore, intended for decl caching in typechecker workers.
/// The key type is intended to be a `Symbol` or tuple of `Symbol`s, and the
/// value type is intended to be a ref-counted pointer (like `Rc`).
pub trait LocalStore<K: Copy, V>: Debug {
    fn get(&self, key: K) -> Option<V>;
    fn insert(&mut self, key: K, val: V);
}
