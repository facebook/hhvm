// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hh24_types::DeclHash;
use hh24_types::FileDeclsHash;

mod datatypes;
mod naming_sqlite;
mod naming_table;
mod summary;

pub use datatypes::FileInfoId;
pub use datatypes::SaveResult;
pub use naming_sqlite::Names;
pub use naming_table::NamingTable;
pub use summary::DeclSummary;
pub use summary::FileSummary;
pub use summary::SymbolRow;

pub fn hash_decl(decl: &oxidized_by_ref::shallow_decl_defs::Decl<'_>) -> DeclHash {
    DeclHash::from_u64(hh_hash::hash(decl))
}

fn hash_decls(decls: &oxidized_by_ref::direct_decl_parser::Decls<'_>) -> FileDeclsHash {
    FileDeclsHash::from_u64(hh_hash::position_insensitive_hash(&decls))
}
