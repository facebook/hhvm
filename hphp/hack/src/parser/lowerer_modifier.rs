// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized::aast::Visibility;
use oxidized::aast_defs::UseAsVisibility;
use parser_rust::token_kind::TokenKind as TK;

#[derive(Copy, Clone, Eq, PartialEq)]
pub struct Kind(u32);
pub const FINAL: Kind = Kind(0b000000001u32);
pub const STATIC: Kind = Kind(0b000000010u32);
pub const ABSTRACT: Kind = Kind(0b000000100u32);
pub const PRIVATE: Kind = Kind(0b000001000u32);
pub const PUBLIC: Kind = Kind(0b000010000u32);
pub const PROTECTED: Kind = Kind(0b000100000u32);
pub const VAR: Kind = Kind(0b001000000u32);
pub const ASYNC: Kind = Kind(0b010000000u32);
pub const COROUTINE: Kind = Kind(0b100000000u32);

pub fn from_token_kind(t: TK) -> Option<Kind> {
    match t {
        TK::Final => Some(FINAL),
        TK::Static => Some(STATIC),
        TK::Abstract => Some(ABSTRACT),
        TK::Private => Some(PRIVATE),
        TK::Public => Some(PUBLIC),
        TK::Protected => Some(PROTECTED),
        TK::Var => Some(VAR),
        TK::Async => Some(ASYNC),
        TK::Coroutine => Some(COROUTINE),
        _ => None,
    }
}

pub fn to_visibility(kind: Kind) -> Option<Visibility> {
    match kind {
        PUBLIC => Some(Visibility::Public),
        PRIVATE => Some(Visibility::Private),
        PROTECTED => Some(Visibility::Protected),
        _ => None,
    }
}

pub fn to_use_as_visibility(kind: Kind) -> Option<UseAsVisibility> {
    use UseAsVisibility::*;
    match kind {
        PUBLIC => Some(UseAsPublic),
        PRIVATE => Some(UseAsPrivate),
        PROTECTED => Some(UseAsProtected),
        FINAL => Some(UseAsFinal),
        _ => None,
    }
}

#[derive(Copy, Clone)]
pub struct KindSet(u32);

pub const VISIBILITIES: KindSet = KindSet(PRIVATE.0 | PUBLIC.0 | PROTECTED.0);

pub const USE_AS_VISIBILITY: KindSet = KindSet(PRIVATE.0 | PUBLIC.0 | PROTECTED.0 | FINAL.0);

impl KindSet {
    pub fn new() -> Self {
        KindSet(0)
    }

    pub fn from_kinds(kinds: &[Kind]) -> Self {
        kinds.iter().fold(KindSet(0), |mut acc, x| {
            acc.add(*x);
            acc
        })
    }

    pub fn add(&mut self, kind: Kind) {
        self.0 = self.0 | kind.0
    }

    pub fn has(&self, kind: Kind) -> bool {
        self.0 & kind.0 > 0
    }

    pub fn has_any(&self, set: KindSet) -> bool {
        self.0 & set.0 > 0
    }
}
