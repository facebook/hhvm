// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<564a20aaccfb840fa9b957b355382b86>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

#![allow(unused_variables)]
use super::type_params::Params;
use super::visitor::Visitor;
pub trait Node<P: Params> {
    fn accept<'node>(
        &'node self,
        ctx: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.recurse(ctx, v)
    }
    fn recurse<'node>(
        &'node self,
        ctx: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        Ok(())
    }
}
