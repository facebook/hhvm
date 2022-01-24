// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<402d57383b3308fdfb81350af7efb994>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

#![allow(unused_variables)]
use super::type_params::Params;
use super::visitor_mut::VisitorMut;
pub trait NodeMut<P: Params> {
    fn accept<'node>(
        &'node mut self,
        ctx: &mut P::Context,
        v: &mut dyn VisitorMut<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.recurse(ctx, v)
    }
    fn recurse<'node>(
        &'node mut self,
        ctx: &mut P::Context,
        v: &mut dyn VisitorMut<'node, Params = P>,
    ) -> Result<(), P::Error> {
        Ok(())
    }
}
