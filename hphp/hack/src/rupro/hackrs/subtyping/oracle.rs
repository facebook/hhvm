// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::{typing::typing_error::Result, typing_decl_provider::Class};
use pos::TypeName;
use std::{fmt::Debug, rc::Rc};
use ty::reason::Reason;
pub trait Oracle<R: Reason>: Debug {
    /// Get a class, return `None` if it can't be found.
    fn get_class(&self, name: TypeName) -> Result<Option<Rc<dyn Class<R>>>>;
}

#[derive(Debug)]
pub struct NoClasses;

impl<R: Reason> Oracle<R> for NoClasses {
    fn get_class(&self, _name: TypeName) -> Result<Option<Rc<dyn Class<R>>>> {
        Ok(None)
    }
}
