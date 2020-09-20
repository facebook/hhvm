// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<d34a4724c5458d84c88da31216c7b067>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

#![allow(unused_variables)]
use super::type_params::Params;
use super::visitor::Visitor;
pub trait Node<P: Params> {
    fn accept<'node>(
        &'node self,
        ctx: &mut P::Context,
        v: &mut dyn Visitor<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.recurse(ctx, v)
    }
    fn recurse<'node>(
        &'node self,
        ctx: &mut P::Context,
        v: &mut dyn Visitor<'node, P = P>,
    ) -> Result<(), P::Error> {
        Ok(())
    }
}
