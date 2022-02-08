// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized_by_ref::file_info::NameType;
use oxidized_by_ref::shallow_decl_defs::Decl;

#[derive(Debug)]
pub enum DeclProvider<'decl> {
    ExternalDeclProvider(external_decl_provider::ExternalDeclProvider<'decl>),
    NoDeclProvider(decl_provider::NoDeclProvider),
}

impl<'decl> decl_provider::DeclProvider<'decl> for DeclProvider<'decl> {
    fn get_decl(&self, kind: NameType, sym: &str) -> Result<Decl<'decl>, decl_provider::Error> {
        match self {
            DeclProvider::ExternalDeclProvider(e) => e.get_decl(kind, sym),
            DeclProvider::NoDeclProvider(e) => e.get_decl(kind, sym),
        }
    }
}
