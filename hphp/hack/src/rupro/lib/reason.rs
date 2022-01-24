// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::hash::Hash;

use ocamlrep::{Allocator, OpaqueValue, ToOcamlRep};

use crate::pos::{BPos, NPos, Pos};

pub trait Reason: Eq + Hash + Clone + ToOcamlRep + std::fmt::Debug {
    /// Position type.
    type Pos: Pos;

    fn mk(cons: &dyn Fn() -> ReasonImpl<Self::Pos>) -> Self;

    fn pos(&self) -> &Self::Pos;

    fn to_oxidized(&self) -> oxidized::typing_reason::Reason;
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum ReasonImpl<P: Pos> {
    Rnone,
    Rwitness(P),
    RwitnessFromDecl(P),
    Rhint(P),
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct BReason(Box<ReasonImpl<BPos>>);

impl Reason for BReason {
    type Pos = BPos;

    fn mk(cons: &dyn Fn() -> ReasonImpl<Self::Pos>) -> Self {
        let x = cons();
        Self(Box::new(x))
    }

    fn pos(&self) -> &BPos {
        use ReasonImpl::*;
        match &*self.0 {
            Rnone => unimplemented!(),
            Rwitness(p) | RwitnessFromDecl(p) | Rhint(p) => p,
        }
    }

    fn to_oxidized(&self) -> oxidized::typing_reason::Reason {
        unimplemented!()
    }
}

impl ToOcamlRep for BReason {
    fn to_ocamlrep<'a, A: Allocator>(&self, _alloc: &'a A) -> OpaqueValue<'a> {
        unimplemented!()
    }
}

/// A stateless sentinal Reason.
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct NReason;

impl Reason for NReason {
    type Pos = NPos;

    fn mk(_cons: &dyn Fn() -> ReasonImpl<Self::Pos>) -> Self {
        NReason
    }

    fn pos(&self) -> &NPos {
        &NPos
    }

    fn to_oxidized(&self) -> oxidized::typing_reason::Reason {
        oxidized::typing_reason::Reason::Rnone
    }
}

impl ToOcamlRep for NReason {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> OpaqueValue<'a> {
        self.to_oxidized().to_ocamlrep(alloc)
    }
}
