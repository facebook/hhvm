// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<c54f4c07553c5395facf70e68eddf9ea>>
//
// To regenerate this file, run:
//   buck run @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:oxidized_regen

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
