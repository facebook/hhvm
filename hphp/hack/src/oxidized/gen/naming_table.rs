// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<dea3618dfc033f9ef66a40d8d0bed7ac>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::OcamlRep;
use serde::Deserialize;
use serde::Serialize;

use crate::file_info;
use crate::relative_path;

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub enum ForwardNamingTableDelta<A> {
    Modified(A),
    Deleted,
}

pub type FileDeltas = relative_path::map::Map<ForwardNamingTableDelta<file_info::FileInfo>>;

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub struct LocalChanges {
    pub file_deltas: FileDeltas,
    pub base_content_version: String,
}

pub type ChangesSinceBaseline = LocalChanges;

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub enum NamingTable {
    Unbacked(relative_path::map::Map<file_info::FileInfo>),
    Backed(LocalChanges),
}

pub type Fast = relative_path::map::Map<file_info::Names>;

pub type SavedStateInfo = relative_path::map::Map<file_info::Saved>;

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub struct SaveResult {
    pub files_added: isize,
    pub symbols_added: isize,
}

#[derive(Clone, Copy, Debug, Deserialize, Eq, OcamlRep, PartialEq, Serialize)]
pub enum TypeOfType {
    TClass = 0,
    TTypedef = 1,
    TRecordDef = 2,
}

#[derive(Clone, Copy, Debug, Deserialize, Eq, OcamlRep, PartialEq, Serialize)]
pub enum BlockedEntry {
    Blocked,
}
