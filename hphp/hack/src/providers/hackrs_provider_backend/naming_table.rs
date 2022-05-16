// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use naming_provider::NamingProvider;
use pos::RelativePath;

pub use naming_provider::Result;

#[derive(Debug)]
pub struct NamingTable(());

impl NamingTable {
    pub fn new() -> Self {
        Self(())
    }
}

impl NamingProvider for NamingTable {
    fn get_type_path(&self, _name: pos::TypeName) -> Result<Option<RelativePath>> {
        todo!()
    }
    fn get_fun_path(&self, _name: pos::FunName) -> Result<Option<RelativePath>> {
        todo!()
    }
    fn get_const_path(&self, _name: pos::ConstName) -> Result<Option<RelativePath>> {
        todo!()
    }
}
