// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cell::RefCell;
use std::rc::Rc;

use eq_modulo_pos::EqModuloPos;
use eq_modulo_pos::EqModuloPosAndReason;
use ocamlrep_derive::FromOcamlRep;
use ocamlrep_derive::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[derive(
    Debug,
    Copy,
    Clone,
    Hash,
    PartialEq,
    Eq,
    EqModuloPos,
    EqModuloPosAndReason,
    PartialOrd,
    Ord
)]
#[derive(Serialize, Deserialize)]
#[derive(ToOcamlRep, FromOcamlRep)]
pub struct Ident(u64);

impl From<u64> for Ident {
    fn from(x: u64) -> Self {
        Self(x)
    }
}

impl From<isize> for Ident {
    fn from(x: isize) -> Self {
        Self(x.try_into().unwrap())
    }
}

impl From<Ident> for isize {
    fn from(x: Ident) -> isize {
        x.0 as isize
    }
}

impl std::fmt::Display for Ident {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "#{}", self.0)
    }
}

#[derive(Debug, Clone)]
pub struct IdentGen {
    next: Rc<RefCell<u64>>,
}

impl IdentGen {
    pub fn new() -> Self {
        Self {
            next: Rc::new(RefCell::new(1)),
        }
    }

    pub fn make(&self) -> Ident {
        let mut r = self.next.borrow_mut();
        let v = *r;
        *r += 1;
        Ident(v)
    }
}
