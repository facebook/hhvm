// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! Provides `MemoizationCache`, a simple cache designed to aid implementation
//! of the `Allocator` trait.

use std::cell::RefCell;

use nohash_hasher::IntMap;

/// A simple scoped cache for memoizing conversions from one pointer-sized value
/// to another. Useful for memoizing conversions between OCaml values and Rust
/// references.
pub struct MemoizationCache {
    /// Maps from input address -> size_in_bytes -> output
    cache: RefCell<Option<IntMap<usize, IntMap<usize, usize>>>>,
}

impl MemoizationCache {
    #[inline(always)]
    pub fn new() -> Self {
        Self {
            cache: RefCell::new(None),
        }
    }

    #[inline(always)]
    pub fn with_cache<T>(&self, f: impl FnOnce() -> T) -> T {
        // The `replace` below should not panic because the only borrows of
        // `self.cache` are in this function and in `memoized`. In both
        // functions, we do not hold a `Ref` or `RefMut` while calling into code
        // which might attempt to re-enter `memoized` or `with_cache`.
        let prev_value = self.cache.replace(Some(IntMap::default()));
        if prev_value.is_some() {
            panic!(
                "Attempted to re-enter MemoizationCache::with_cache \
                (probably via ocamlrep::Allocator::add_root, which is not re-entrant)"
            );
        }
        let result = f();
        // As above, this `replace` should not panic.
        self.cache.replace(None);
        result
    }

    #[inline(always)]
    pub fn memoized(&self, input: usize, size_in_bytes: usize, f: impl FnOnce() -> usize) -> usize {
        if size_in_bytes == 0 {
            return f();
        }
        let memoized_output = match self.cache.borrow().as_ref().map(|cache| {
            cache
                .get(&input)
                .and_then(|m| m.get(&size_in_bytes).copied())
        }) {
            None => return f(),
            Some(output) => output,
        };
        match memoized_output {
            Some(output) => output,
            None => {
                let output = f();
                // The `borrow_mut` below should not panic because the only
                // borrows of `self.cache` are in this function and in
                // `with_cache`. In this function, we do not hold a `Ref` or
                // `RefMut` while calling into `f` (or any other function which
                // might attempt to re-enter this function or `with_cache`).
                let mut cache = self.cache.borrow_mut();
                // The `unwrap` below should not panic. We know `self.cache` was
                // not None upon entering this function because we looked up
                // `memoized_output`. The only function which can replace the
                // cache with None is `with_cache`, which would have panicked if
                // `f` attempted to re-enter it.
                let by_size = cache.as_mut().unwrap().entry(input).or_default();
                by_size.insert(size_in_bytes, output);
                output
            }
        }
    }
}
