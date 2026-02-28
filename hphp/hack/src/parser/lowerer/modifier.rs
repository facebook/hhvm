// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized::aast::Visibility;
use parser_core_types::token_kind::TokenKind as TK;

#[derive(Copy, Clone, Eq, PartialEq)]
pub struct Kind(u32);
pub const XHP: Kind = Kind(1u32);
pub const FINAL: Kind = Kind(1u32 << 1);
pub const STATIC: Kind = Kind(1u32 << 2);
pub const ABSTRACT: Kind = Kind(1u32 << 3);
pub const PRIVATE: Kind = Kind(1u32 << 4);
pub const PUBLIC: Kind = Kind(1u32 << 5);
pub const PROTECTED: Kind = Kind(1u32 << 6);
pub const VAR: Kind = Kind(1u32 << 7);
pub const ASYNC: Kind = Kind(1u32 << 8);
pub const READONLY: Kind = Kind(1u32 << 9);
pub const INTERNAL: Kind = Kind(1u32 << 10);
pub const PROTECTED_INTERNAL: Kind = Kind(1u32 << 11);

pub fn from_token_kind(t: TK) -> Option<Kind> {
    match t {
        TK::Final => Some(FINAL),
        TK::Static => Some(STATIC),
        TK::Abstract => Some(ABSTRACT),
        TK::Private => Some(PRIVATE),
        TK::Public => Some(PUBLIC),
        TK::Protected => Some(PROTECTED),
        TK::Internal => Some(INTERNAL),
        TK::Var => Some(VAR),
        TK::Async => Some(ASYNC),
        TK::XHP => Some(XHP),
        TK::Readonly => Some(READONLY),
        _ => None,
    }
}

pub fn to_visibility(kind: Kind) -> Option<Visibility> {
    match kind {
        PUBLIC => Some(Visibility::Public),
        PRIVATE => Some(Visibility::Private),
        PROTECTED => Some(Visibility::Protected),
        INTERNAL => Some(Visibility::Internal),
        PROTECTED_INTERNAL => Some(Visibility::ProtectedInternal),
        _ => None,
    }
}

#[derive(Copy, Clone)]
pub struct KindSet(u32);

impl KindSet {
    pub fn new() -> Self {
        KindSet(0)
    }

    pub fn add(&mut self, kind: Kind) {
        self.0 |= kind.0
    }

    pub fn has(&self, kind: Kind) -> bool {
        self.0 & kind.0 > 0
    }
}
