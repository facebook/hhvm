// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::aast_defs::*;
use crate::pos::Pos;
use std::boxed::Box;

impl Lid {
    pub fn new(p: Pos, s: String) -> Self {
        Self(p, (0, s))
    }

    pub fn from_counter(p: Pos, counter: isize, s: &str) -> Self {
        Self(p, (counter, String::from(s)))
    }
}

impl Hint {
    pub fn new(p: Pos, h: Hint_) -> Self {
        Self(p, Box::new(h))
    }

    pub fn as_happly(&self) -> Option<(&Sid, &Vec<Hint>)> {
        self.1.as_happly()
    }

    pub fn is_hlike(&self) -> bool {
        self.1.is_hlike()
    }
}

impl Hint_ {
    pub fn is_hfun(&self) -> bool {
        match self {
            Hint_::Hfun(_) => true,
            _ => false,
        }
    }

    pub fn as_happly(&self) -> Option<(&Sid, &Vec<Hint>)> {
        match self {
            Hint_::Happly(x, y) => Some((x, y)),
            _ => None,
        }
    }

    pub fn is_happly(&self) -> bool {
        self.as_happly().is_some()
    }

    pub fn is_hlike(&self) -> bool {
        match self {
            Hint_::Hlike(_) => true,
            _ => false,
        }
    }
}

impl Lid {
    pub fn name(&self) -> &String {
        crate::local_id::get_name(&self.1)
    }
}
