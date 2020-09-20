// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fmt;

pub type Id = usize;

#[derive(Debug, Clone, PartialEq, Eq, std::cmp::Ord, std::cmp::PartialOrd)]
pub enum Label {
    Regular(Id),
    DefaultArg(Id),
    Named(String),
}
impl Label {
    pub fn id(&self) -> Result<&Id, Error> {
        match self {
            Label::Regular(id) => Ok(id),
            Label::DefaultArg(id) => Ok(id),
            Label::Named(_) => Err(Error::Id),
        }
    }

    pub fn map<F: FnOnce(Id) -> Id>(&self, f: F) -> Result<Label, Error> {
        match self {
            Label::Regular(id) => Ok(Label::Regular(f(*id))),
            Label::DefaultArg(id) => Ok(Label::DefaultArg(f(*id))),
            Label::Named(_) => Err(Error::Map),
        }
    }

    pub fn map_mut<F: FnOnce(&mut Id) -> ()>(&mut self, f: F) {
        match self {
            Label::Regular(id) | Label::DefaultArg(id) => f(id),
            Label::Named(_) => panic!("Label should be rewritten before this point"),
        }
    }

    pub fn option_map<F: FnOnce(Id) -> Option<Id>>(&self, f: F) -> Result<Option<Label>, Error> {
        match self {
            Label::Regular(id) => {
                if let Some(l) = f(*id) {
                    return Ok(Some(Label::Regular(l)));
                }
            }
            Label::DefaultArg(id) => {
                if let Some(l) = f(*id) {
                    return Ok(Some(Label::DefaultArg(l)));
                }
            }
            Label::Named(_) => return Err(Error::OptionMap),
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
impl fmt::Display for Error {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
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

    /// Produces named label "resumeN" for use by fuzzer, where N is the next unused Id.
    pub fn next_resume(&mut self) -> Label {
        Label::Named(format!("resume{}", self.get_next()))
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
