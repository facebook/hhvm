// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::RelativePath;
use std::hash::Hash;

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
    /// Make a new instance. If the implementing Pos is stateful,
    /// it will call cons() to obtain interned values to construct the instance.
    fn mk(cons: impl FnOnce() -> (RelativePath, FilePos, FilePos)) -> Self;

    fn to_oxidized_pos(&self) -> oxidized::pos::Pos;
}

/// Represents a closed-ended range [start, end] in a file.
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
struct PosImpl {
    file: RelativePath,
    start: FilePos,
    end: FilePos,
}

// Putting the contents behind a Box helps ensure that we don't blow up the
// sizes of all our types containing positions (particularly enums, where we pay
// for the size of the position even in variants which don't contain a
// position).
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct BPos(Box<PosImpl>);

impl Pos for BPos {
    fn mk(cons: impl FnOnce() -> (RelativePath, FilePos, FilePos)) -> Self {
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
    fn mk(_cons: impl FnOnce() -> (RelativePath, FilePos, FilePos)) -> Self {
        NPos
    }

    fn to_oxidized_pos(&self) -> oxidized::pos::Pos {
        oxidized::pos::Pos::make_none()
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct Positioned<S, P> {
    // Caution: field order will matter if we ever derive
    // `ToOcamlRep`/`FromOcamlRep` for this type.
    pos: P,
    id: S,
}

impl<S, P> Positioned<S, P> {
    pub fn new(pos: P, id: S) -> Self {
        Self { pos, id }
    }

    pub fn pos(&self) -> &P {
        &self.pos
    }

    pub fn id_ref(&self) -> &S {
        &self.id
    }
}

impl<S: Copy, P> Positioned<S, P> {
    pub fn id(&self) -> S {
        self.id
    }
}

impl<S: ToString, P: Pos> Positioned<S, P> {
    pub fn to_oxidized(&self) -> oxidized::typing_reason::PosId {
        (self.pos.to_oxidized_pos(), self.id.to_string())
    }
}
