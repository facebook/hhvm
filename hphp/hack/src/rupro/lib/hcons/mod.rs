// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::borrow::ToOwned;
use std::collections::{hash_map::Entry, HashMap};
use std::fmt;
use std::hash::{Hash, Hasher};
use std::ops::{Deref, DerefMut};
use std::rc::{Rc, Weak};
use std::sync::atomic::{AtomicU64, Ordering};
use std::sync::Mutex;

struct HcImpl<T> {
    node: T,
    tag: u64,
}

impl<T: fmt::Debug> fmt::Debug for HcImpl<T> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> Result<(), fmt::Error> {
        self.node.fmt(f)
    }
}

impl<T> Deref for HcImpl<T> {
    type Target = T;

    fn deref(&self) -> &T {
        &self.node
    }
}

impl<T> DerefMut for HcImpl<T> {
    fn deref_mut(&mut self) -> &mut T {
        &mut self.node
    }
}

impl<T> Eq for HcImpl<T> {}

impl<T> Hash for HcImpl<T> {
    fn hash<H: Hasher>(&self, state: &mut H) {
        self.tag.hash(state);
    }
}

impl<T> PartialEq for HcImpl<T> {
    fn eq(&self, other: &Self) -> bool {
        self.tag == other.tag
    }
}

/// A hash-consed pointer.
#[derive(Clone)]
pub struct Hc<T>(Rc<HcImpl<T>>);

impl<T: fmt::Debug> fmt::Debug for Hc<T> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> Result<(), fmt::Error> {
        self.0.fmt(f)
    }
}

impl<T> Deref for Hc<T> {
    type Target = T;

    fn deref(&self) -> &T {
        &self.0
    }
}

impl<T> Eq for Hc<T> {}

impl<T> Hash for Hc<T> {
    fn hash<H: Hasher>(&self, state: &mut H) {
        self.0.hash(state);
    }
}

impl<T> PartialEq for Hc<T> {
    fn eq(&self, other: &Self) -> bool {
        self.0 == other.0
    }
}

#[derive(Debug)]
pub struct Conser<T> {
    next_tag: AtomicU64,
    table: Mutex<HashMap<T, Weak<HcImpl<T>>>>,
}

impl<T: Eq + Hash + Clone> Conser<T> {
    pub fn new() -> Self {
        Conser {
            next_tag: AtomicU64::new(0),
            table: Mutex::new(HashMap::new()),
        }
    }

    pub fn mk(&self, x: T) -> Hc<T> {
        let make_rc = |x: &T| {
            let node = x.to_owned();
            let tag = self.next_tag.fetch_add(1, Ordering::Relaxed);
            Rc::new(HcImpl { node, tag })
        };
        let mut table = self.table.lock().unwrap();
        let rc = match table.entry(x) {
            Entry::Occupied(mut o) => match o.get().upgrade() {
                Some(rc) => rc,
                None => {
                    let rc = make_rc(o.key());
                    o.insert(Rc::downgrade(&rc));
                    rc
                }
            },
            Entry::Vacant(v) => {
                let rc = make_rc(v.key());
                v.insert(Rc::downgrade(&rc));
                rc
            }
        };
        Hc(rc)
    }

    pub fn gc(&self) -> bool {
        let mut table = self.table.lock().unwrap();
        let mut flag = false;
        loop {
            let l = table.len();
            table.retain(|_, v| v.strong_count() != 0);
            if l == table.len() {
                break;
            }
            flag = true;
        }
        flag
    }
}
