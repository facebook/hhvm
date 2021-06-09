// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized_by_ref::shallow_decl_defs::Decl;

#[derive(Debug)]
pub enum Error {
    NotFound,
}

pub trait DeclProvider<'decl> {
    fn get_decl(&self, symbol: &str) -> Result<Decl<'decl>, Error>;
}

#[derive(Debug, Default)]
pub struct NoDeclProvider;

impl<'decl> DeclProvider<'decl> for NoDeclProvider {
    fn get_decl(&self, _: &str) -> Result<Decl<'decl>, Error> {
        Err(Error::NotFound)
    }
}
