// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#[derive(Debug, Clone)]
pub enum FileMode {
    Hhi,
    Strict,
}

#[derive(Debug, Clone)]
pub enum FilePos {
    Full(oxidized::pos::Pos),
}

#[derive(Debug, Clone)]
pub struct FileId {
    pub pos: FilePos,
    pub id: String,
    pub hash: Option<u64>,
}

impl FileId {
    pub fn with_full_pos(pos: oxidized::pos::Pos, name: String, hash: Option<u64>) -> Self {
        Self {
            pos: FilePos::Full(pos),
            id: name,
            hash,
        }
    }
}

#[derive(Debug, Clone)]
pub struct FileInfo {
    pub hash: Option<u64>,
    pub file_mode: Option<FileMode>,
    pub funs: Vec<FileId>,
    pub classes: Vec<FileId>,
    pub record_defs: Vec<FileId>,
    pub typedefs: Vec<FileId>,
    pub consts: Vec<FileId>,
}

impl FileInfo {
    pub fn empty() -> Self {
        Self {
            hash: None,
            file_mode: None,
            funs: vec![],
            classes: vec![],
            record_defs: vec![],
            typedefs: vec![],
            consts: vec![],
        }
    }
}
