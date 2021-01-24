// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<a42808e6c232d7ee6f271188854adbc8>>
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
