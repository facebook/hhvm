// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![allow(unused)]
use std::borrow::{Borrow, ToOwned};
use std::collections::HashMap;
use std::fmt;
use std::hash::{Hash, Hasher};
use std::ops::{Deref, DerefMut};
use std::rc::{Rc, Weak};
use std::sync::atomic::{AtomicU64, Ordering};
use std::sync::Mutex;

struct ConsedImpl<T> {
    node: T,
    tag: u64,
}

impl<T: fmt::Debug> fmt::Debug for ConsedImpl<T> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> Result<(), fmt::Error> {
        self.node.fmt(f)
    }
}

impl<T> Deref for ConsedImpl<T> {
    type Target = T;

    fn deref(&self) -> &T {
        &self.node
    }
}

impl<T> DerefMut for ConsedImpl<T> {
    fn deref_mut(&mut self) -> &mut T {
        &mut self.node
    }
}

impl<T> Eq for ConsedImpl<T> {}

impl<T> Hash for ConsedImpl<T> {
    fn hash<H: Hasher>(&self, state: &mut H) {
        self.tag.hash(state);
    }
}

impl<T> PartialEq for ConsedImpl<T> {
    fn eq(&self, other: &Self) -> bool {
        self.tag == other.tag
    }
}

#[derive(Clone)]
pub struct Consed<T>(Rc<ConsedImpl<T>>);

impl<T: fmt::Debug> fmt::Debug for Consed<T> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> Result<(), fmt::Error> {
        self.0.fmt(f)
    }
}

impl<T> Deref for Consed<T> {
    type Target = T;

    fn deref(&self) -> &T {
        &self.0
    }
}

impl<T> Eq for Consed<T> {}

impl<T> Hash for Consed<T> {
    fn hash<H: Hasher>(&self, state: &mut H) {
        self.0.hash(state);
    }
}

impl<T> PartialEq for Consed<T> {
    fn eq(&self, other: &Self) -> bool {
        self.0 == other.0
    }
}

#[derive(Debug)]
pub struct Conser<T> {
    next_tag: AtomicU64,
    table: Mutex<HashMap<T, Weak<ConsedImpl<T>>>>,
}

impl<T: Eq + Hash + Clone> Conser<T> {
    pub fn new() -> Self {
        Conser {
            next_tag: AtomicU64::new(0),
            table: Mutex::new(HashMap::new()),
        }
    }

    pub fn mk<Q: ?Sized>(&self, x: &Q) -> Consed<T>
    where
        T: Borrow<Q>,
        Q: ToOwned<Owned = T> + Hash + Eq,
    {
        let mut table = self.table.lock().unwrap();
        let rc = table.get(x.borrow()).and_then(Weak::upgrade);
        match rc {
            Some(rc) => Consed(rc),
            None => {
                let x = x.to_owned();
                let tag = self.next_tag.fetch_add(1, Ordering::Relaxed);
                let consed = ConsedImpl {
                    node: x.clone(),
                    tag,
                };
                let consed = Rc::new(consed);
                table.insert(x, Rc::downgrade(&consed));
                Consed(consed)
            }
        }
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
