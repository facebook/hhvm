// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hh24_types::DeclHash;
use hh24_types::FileDeclsHash;
use hh24_types::ToplevelCanonSymbolHash;
use hh24_types::ToplevelSymbolHash;

mod datatypes;
mod naming_sqlite;
mod naming_table;
mod summary;

pub use datatypes::FileInfoId;
pub use naming_sqlite::Names;
pub use naming_table::NamingTable;
pub use summary::DeclSummary;
pub use summary::FileSummary;
pub use summary::SymbolRow;

fn hash_decl(decl: &oxidized_by_ref::shallow_decl_defs::Decl<'_>) -> DeclHash {
    use oxidized_by_ref::shallow_decl_defs::Decl;
    match *decl {
        Decl::Class(decl) => DeclHash::from_u64(hh_hash::hash(decl)),
        Decl::Const(decl) => DeclHash::from_u64(hh_hash::hash(decl)),
        Decl::Fun(decl) => DeclHash::from_u64(hh_hash::hash(decl)),
        Decl::Typedef(decl) => DeclHash::from_u64(hh_hash::hash(decl)),
        Decl::Module(decl) => DeclHash::from_u64(hh_hash::hash(decl)),
    }
}

fn hash_decls(decls: &oxidized_by_ref::direct_decl_parser::Decls<'_>) -> FileDeclsHash {
    FileDeclsHash::from_u64(hh_hash::hash(&decls))
}

pub fn hash_name(name: &str, name_type: oxidized::file_info::NameType) -> ToplevelSymbolHash {
    use oxidized::file_info::NameType;
    match name_type {
        NameType::Fun => ToplevelSymbolHash::from_fun(name),
        NameType::Const => ToplevelSymbolHash::from_const(name),
        NameType::Class | NameType::Typedef => ToplevelSymbolHash::from_type(name),
        NameType::Module => ToplevelSymbolHash::from_module(name),
    }
}

pub fn hash_canon_name(
    name: String,
    name_type: oxidized::file_info::NameType,
) -> ToplevelCanonSymbolHash {
    use oxidized::file_info::NameType;
    match name_type {
        NameType::Fun => ToplevelCanonSymbolHash::from_fun(name),
        NameType::Const => ToplevelCanonSymbolHash::from_const(name),
        NameType::Class | NameType::Typedef => ToplevelCanonSymbolHash::from_type(name),
        NameType::Module => ToplevelCanonSymbolHash::from_module(name),
    }
}
