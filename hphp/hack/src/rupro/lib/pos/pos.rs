// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::hash::Hash;

use crate::pos::{RelativePath, Symbol};

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct FilePos {
    /// 1-indexed line number.
    pub lnum: u64,

    /// Byte offset from start of file to start of line.
    pub bol: u64,

    /// Byte offset from start of line.
    pub cnum: u64,
}

pub trait Pos: Eq + Hash + Clone + std::fmt::Debug {
    fn mk(cons: &dyn Fn() -> (RelativePath, FilePos, FilePos)) -> Self;

    fn to_oxidized_pos(&self) -> oxidized::pos::Pos;
}

/// Represents a closed-ended range [start, end] in a file.
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
struct PosImpl {
    file: RelativePath,
    start: FilePos,
    end: FilePos,
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct BPos(Box<PosImpl>);

impl Pos for BPos {
    fn mk(cons: &dyn Fn() -> (RelativePath, FilePos, FilePos)) -> Self {
        let (file, start, end) = cons();
        Self(Box::new(PosImpl { file, start, end }))
    }

    fn to_oxidized_pos(&self) -> oxidized::pos::Pos {
        unimplemented!()
    }
}

/// A stateless sentinal Pos.
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct NPos;

impl Pos for NPos {
    fn mk(_cons: &dyn Fn() -> (RelativePath, FilePos, FilePos)) -> Self {
        NPos
    }

    fn to_oxidized_pos(&self) -> oxidized::pos::Pos {
        oxidized::pos::Pos::make_none()
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct PosId<P: Pos>(P, Symbol);

impl<P: Pos> PosId<P> {
    pub fn new(pos: P, id: Symbol) -> Self {
        Self(pos, id)
    }

    pub fn pos(&self) -> &P {
        &self.0
    }

    pub fn id(&self) -> &Symbol {
        &self.1
    }

    pub fn to_oxidized(&self) -> oxidized::typing_reason::PosId {
        (self.pos().to_oxidized_pos(), self.id().to_string())
    }
}
