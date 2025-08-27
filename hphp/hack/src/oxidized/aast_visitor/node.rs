// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<785dad7d9eb90efc05b70706581062ad>>
//
// To regenerate this file, run:
//   buck run @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:oxidized_regen

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
