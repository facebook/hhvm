// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::reason::Reason;

use super::Allocator;

impl<R: Reason> Allocator<R> {
    pub fn reason(&self, reason: &oxidized_by_ref::typing_reason::T_<'_>) -> R {
        R::mk(&|| {
            use crate::reason::ReasonImpl as RI;
            use oxidized_by_ref::typing_reason::T_ as OR;
            match reason {
                OR::Rnone => RI::Rnone,
                OR::Rwitness(pos) => RI::Rwitness(self.pos_from_decl(pos)),
                OR::RwitnessFromDecl(pos) => RI::RwitnessFromDecl(self.pos_from_decl(pos)),
                OR::Rhint(pos) => RI::Rhint(self.pos_from_decl(pos)),
                r => unimplemented!("reason: {:?}", r),
            }
        })
    }
}
