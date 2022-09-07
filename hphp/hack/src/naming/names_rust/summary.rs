// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hh24_types::DeclHash;
use hh24_types::FileDeclsHash;
use hh24_types::ToplevelCanonSymbolHash;
use hh24_types::ToplevelSymbolHash;
use oxidized::file_info;
use oxidized::file_info::NameType;
use oxidized::relative_path::RelativePath;
use oxidized_by_ref::direct_decl_parser::Decl;
use oxidized_by_ref::direct_decl_parser::ParsedFile;

/// Similar to `oxidized::file_info::FileInfo`, but containing only the
/// information which is necessary to populate the naming table (i.e., omitting
/// positions). Includes hashes of individual decls.
#[derive(Clone, Debug, Hash)]
pub struct FileSummary {
    pub mode: Option<file_info::Mode>,
    pub hash: FileDeclsHash,
    pub decls: Vec<DeclSummary>,
}

impl FileSummary {
    pub fn from_decls(file: ParsedFile<'_>) -> Self {
        Self {
            mode: file.mode,
            hash: FileDeclsHash::from(file.decls),
            decls: file.decls.iter().map(DeclSummary::from).collect(),
        }
    }

    pub fn example_symbol(&self) -> Option<String> {
        self.decls.first().map(|decl| &decl.symbol).cloned()
    }

    pub fn decl_hashes(&self) -> impl Iterator<Item = (ToplevelSymbolHash, DeclHash)> + '_ {
        self.decls
            .iter()
            .map(|decl| (decl.symbol_hash(), decl.hash))
    }

    pub fn symbols_of_kind(&self, kind: NameType) -> impl Iterator<Item = (&str, DeclHash)> + '_ {
        self.decls
            .iter()
            .filter(move |decl| decl.name_type == kind)
            .map(|decl| (decl.symbol.as_str(), decl.hash))
    }

    pub fn classes(&self) -> impl Iterator<Item = (&str, DeclHash)> + '_ {
        self.symbols_of_kind(NameType::Class)
    }

    pub fn funs(&self) -> impl Iterator<Item = (&str, DeclHash)> + '_ {
        self.symbols_of_kind(NameType::Fun)
    }

    pub fn consts(&self) -> impl Iterator<Item = (&str, DeclHash)> + '_ {
        self.symbols_of_kind(NameType::Const)
    }

    pub fn typedefs(&self) -> impl Iterator<Item = (&str, DeclHash)> + '_ {
        self.symbols_of_kind(NameType::Typedef)
    }
}

#[derive(Clone, Debug, Hash)]
pub struct DeclSummary {
    pub name_type: file_info::NameType,
    pub symbol: String,
    pub hash: DeclHash,
}

impl DeclSummary {
    pub fn new(symbol: &str, decl: Decl<'_>) -> Self {
        Self {
            name_type: decl.kind(),
            symbol: symbol.to_owned(),
            hash: DeclHash::from(decl),
        }
    }

    pub fn symbol_hash(&self) -> ToplevelSymbolHash {
        ToplevelSymbolHash::from(self.name_type.into(), &self.symbol)
    }
}

impl From<(&str, Decl<'_>)> for DeclSummary {
    fn from((symbol, decl): (&str, Decl<'_>)) -> Self {
        Self::new(symbol, decl)
    }
}

/// This is what each row looks like in NAMING_SYMBOLS and NAMING_SYMBOLS_OVERFLOW,
/// i.e. the reverse naming table. When this struct is used for reading from the
/// database, we generally join NAMING_FILE_INFO to populate 'path' since there's very
/// little you can do with a row without turning file_info_id into a pathname;
/// when used for writing, we generally ignore 'path'.
/// This structure is a bit like DeclSummary. The difference is that SymbolRow
/// can be reconstructed from out of the reverse-naming-table (hence can be used
/// by algorithms that want to retrieve then write-back stuff from the table),
/// while DeclSummary can't.
#[derive(Clone, Debug)]
pub struct SymbolRow {
    pub hash: ToplevelSymbolHash,
    pub canon_hash: ToplevelCanonSymbolHash,
    pub decl_hash: DeclHash,
    pub kind: NameType,
    pub file_info_id: crate::FileInfoId,
    pub path: RelativePath,
}

impl SymbolRow {
    /// This method walks the Decl structure to construct a hash for it
    pub fn new(
        path: RelativePath,
        file_info_id: crate::FileInfoId,
        name: &str,
        decl: Decl<'_>,
    ) -> Self {
        let kind = decl.kind();
        Self {
            hash: ToplevelSymbolHash::from(kind.into(), name),
            canon_hash: ToplevelCanonSymbolHash::from(kind.into(), name.to_owned()),
            kind,
            decl_hash: DeclHash::from(decl),
            file_info_id,
            path,
        }
    }
}
