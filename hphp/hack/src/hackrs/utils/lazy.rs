// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! `once_cell::unsync::Lazy` equivalent that takes an arbitrary closure,
//! instead of a function pointer.

use std::cell::Cell;
use std::cell::UnsafeCell;

/// A lazy value that accepts a `dyn FnOnce`.
pub struct Lazy<'a, T> {
    cell: UnsafeCell<Option<T>>,
    init: Cell<Option<&'a dyn Fn() -> T>>,
}

impl<'a, T: std::fmt::Debug> std::fmt::Debug for Lazy<'a, T> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> Result<(), std::fmt::Error> {
        Self::force(self).fmt(f)
    }
}

impl<'a, T: Clone> Clone for Lazy<'a, T> {
    fn clone(&self) -> Self {
        Self {
            cell: UnsafeCell::new(self.get().cloned()),
            init: self.init.clone(),
        }
    }
}

impl<'a, T> Lazy<'a, T> {
    pub fn new(f: &'a dyn Fn() -> T) -> Self {
        Self {
            cell: UnsafeCell::new(None),
            init: Cell::new(Some(f)),
        }
    }

    fn get(&self) -> Option<&T> {
        unsafe { &*self.cell.get() }.as_ref()
    }

    pub fn force(this: &Self) -> &T {
        if let Some(val) = this.get() {
            return val;
        }

        let f = this
            .init
            .take()
            .expect("Lazy value was previously poisoned");
        let val = f();

        let slot = unsafe { &mut *this.cell.get() };
        // Safety: slot was None before.
        *slot = Some(val);

        this.get().unwrap()
    }
}
