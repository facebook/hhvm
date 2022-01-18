// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::cell::RefCell;
use std::collections::HashMap;
use std::rc::Rc;

use crate::parsing_error::ParsingError;
use crate::pos::RelativePath;

pub type AstItem = (oxidized::aast::Program<(), ()>, Vec<ParsingError>);

pub trait AstCache: std::fmt::Debug {
    fn get_ast(&self, path: &RelativePath) -> Option<Rc<AstItem>>;

    fn put_ast(&self, name: RelativePath, cls: Rc<AstItem>);
}

#[derive(Debug)]
pub struct AstLocalCache {
    files: RefCell<HashMap<RelativePath, Rc<AstItem>>>,
}

impl AstLocalCache {
    pub fn new() -> Self {
        Self {
            files: RefCell::new(HashMap::new()),
        }
    }
}

impl AstCache for AstLocalCache {
    fn get_ast(&self, name: &RelativePath) -> Option<Rc<AstItem>> {
        self.files.borrow().get(name).cloned()
    }

    fn put_ast(&self, name: RelativePath, cls: Rc<AstItem>) {
        self.files.borrow_mut().insert(name, cls);
    }
}
