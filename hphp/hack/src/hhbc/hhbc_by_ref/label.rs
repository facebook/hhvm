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
    pub fn id(&self) -> std::result::Result<&Id, Error> {
        match self {
            Label::Regular(id) => Ok(id),
            Label::DefaultArg(id) => Ok(id),
        }
    }

    pub fn map<F: FnOnce(Id) -> Id>(&self, f: F) -> std::result::Result<Label, Error> {
        match *self {
            Label::Regular(id) => Ok(Label::Regular(f(id))),
            Label::DefaultArg(id) => Ok(Label::DefaultArg(f(id))),
        }
    }

    pub fn map_mut<F: FnOnce(&mut Id)>(&mut self, f: F) {
        match *self {
            Label::Regular(ref mut id) | Label::DefaultArg(ref mut id) => f(id),
        }
    }

    pub fn option_map<F: FnOnce(Id) -> Option<Id>>(
        &self,
        f: F,
    ) -> std::result::Result<Option<Label>, Error> {
        match *self {
            Label::Regular(id) => {
                if let Some(l) = f(id) {
                    return Ok(Some(Label::Regular(l)));
                }
            }
            Label::DefaultArg(id) => {
                if let Some(l) = f(id) {
                    return Ok(Some(Label::DefaultArg(l)));
                }
            }
        }
        Ok(None)
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
