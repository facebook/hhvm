// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fmt;

#[derive(Debug, Clone, Copy)]
pub struct Id(usize);

impl fmt::Display for Id {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "Id({})", self.0)
    }
}

#[derive(Debug, Clone)]
pub struct Iter {
    next: Id,
    count: usize,
}
impl Iter {
    pub fn count(&self) -> usize {
        self.count
    }

    pub fn get(&mut self) -> Id {
        let curr = self.next.0;
        self.next.0 = curr + 1;
        self.count = if self.count > self.next.0 {
            self.count
        } else {
            self.next.0
        };
        Id(curr)
    }

    pub fn free(&mut self) {
        self.next.0 -= 1;
    }

    pub fn reset(&mut self) {
        *self = Self::default();
    }

    pub fn store_current_state(&mut self) {
        STORED.with(|x| {
            let stored_mut: &mut Stored = &mut *x.borrow_mut();
            stored_mut.store(self.clone());
        });
    }

    pub fn state_has_changed(&self) -> bool {
        STORED.with(|stored| {
            let (Id(x), Id(y)) = (self.next, (&*stored.borrow()).get_iterator().next);
            x == y
        })
    }

    /// Revert to the old state stored in STORED, and return newly registered
    /// iterators to be freed
    pub fn revert_state(&mut self) -> Vec<Iter> {
        STORED.with(|stored| {
            let old_iterator = (&*stored.borrow()).get_iterator().to_owned();
            let (Id(new_id), Id(old_id)) = (self.next, old_iterator.next);
            let mut iters_to_free = Vec::new();
            for i in old_id..new_id {
                iters_to_free.push(Iter {
                    next: Id(i),
                    count: i + 1,
                });
            }
            *self = old_iterator;
            iters_to_free
        })
    }
}

impl Default for Iter {
    fn default() -> Self {
        Iter {
            next: Id(0),
            count: 0,
        }
    }
}

use ::std::cell::RefCell;
thread_local! {
    static STORED: RefCell<Stored> = RefCell::new( Stored::default() );
}

#[derive(Default)]
struct Stored {
    iterator: Iter,
}

impl Stored {
    fn store(&mut self, iterator: Iter) {
        self.iterator = iterator;
    }

    fn get_iterator(&self) -> &Iter {
        &self.iterator
    }
}
