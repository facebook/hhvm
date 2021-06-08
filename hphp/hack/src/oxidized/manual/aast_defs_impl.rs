// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::pos::Pos;
use crate::{aast_defs::*, ast_defs::*};
use std::{boxed::Box, convert::AsRef};

impl Lid {
    pub fn new(p: Pos, s: String) -> Self {
        Self(p, (0, s))
    }

    pub fn from_counter(p: Pos, counter: isize, s: &str) -> Self {
        Self(p, (counter, String::from(s)))
    }

    pub fn name(&self) -> &String {
        crate::local_id::get_name(&self.1)
    }

    pub fn pos(&self) -> &Pos {
        &self.0
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

impl AsRef<Hint> for Hint {
    fn as_ref(&self) -> &Self {
        self
    }
}

impl AsRef<str> for Id {
    fn as_ref(&self) -> &str {
        &self.1
    }
}

impl AsRef<str> for Visibility {
    fn as_ref(&self) -> &str {
        use Visibility::*;
        match self {
            Private => "private",
            Public => "public",
            Protected => "protected",
            Internal => "internal",
        }
    }
}

impl std::fmt::Display for Visibility {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "{}", self.as_ref())
    }
}
