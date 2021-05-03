// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub type Id = usize;

#[derive(Debug, Clone, Copy, PartialEq, Eq, std::cmp::Ord, std::cmp::PartialOrd)]
pub enum Label {
    Regular(Id),
    DefaultArg(Id),
}
impl Label {
    pub fn id(&self) -> &Id {
        match self {
            Label::Regular(id) => id,
            Label::DefaultArg(id) => id,
        }
    }

    pub fn map<F: FnOnce(&Id) -> Id>(&self, f: F) -> Label {
        match self {
            Label::Regular(id) => Label::Regular(f(&id)),
            Label::DefaultArg(id) => Label::DefaultArg(f(&id)),
        }
    }
}

#[derive(Debug)]
pub enum Error {
    Id,
    OptionMap,
    Map,
}
impl std::fmt::Display for Error {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        let mut msg = "Label should be rewritten before this point".to_owned();
        msg.push_str(match self {
            Error::Id => " (id)",
            Error::OptionMap => " (option_map)",
            Error::Map => " (map)",
        });
        write!(f, "{}", msg)
    }
}

#[derive(Default, Debug)]
pub struct Gen {
    next_id: Id,
}

impl Gen {
    pub fn next_regular(&mut self) -> Label {
        Label::Regular(self.get_next())
    }
    pub fn next_default_arg(&mut self) -> Label {
        Label::DefaultArg(self.get_next())
    }

    fn get_next(&mut self) -> Id {
        let curr_id = self.next_id;
        self.next_id = curr_id + 1;
        curr_id
    }

    pub fn reset(&mut self) {
        self.next_id = 0;
    }
}
