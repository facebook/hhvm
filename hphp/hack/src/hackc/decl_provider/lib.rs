// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized_by_ref::file_info::NameType;
use oxidized_by_ref::shallow_decl_defs::Decl;
use thiserror::Error;

pub type Result<T, E = Error> = std::result::Result<T, E>;

#[derive(Debug, Error)]
pub enum Error {
    #[error("Decl not found")]
    NotFound,
}

pub trait DeclProvider<'decl>: std::fmt::Debug {
    fn get_decl(&self, kind: NameType, symbol: &str) -> Result<Decl<'decl>>;
}

#[derive(Debug, Default)]
pub struct NoDeclProvider;

impl<'decl> DeclProvider<'decl> for NoDeclProvider {
    fn get_decl(&self, _: NameType, _: &str) -> Result<Decl<'decl>> {
        Err(Error::NotFound)
    }
}
