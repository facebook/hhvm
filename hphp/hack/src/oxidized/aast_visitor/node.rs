// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<5b3179e1348a65aed1d03a342cf4970f>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

#![allow(unused_variables)]
use super::type_params::Params;
use super::visitor::Visitor;
pub trait Node<P: Params> {
    fn accept(&self, ctx: &mut P::Context, v: &mut dyn Visitor<P = P>) -> Result<(), P::Error> {
        self.recurse(ctx, v)
    }
    fn recurse(&self, ctx: &mut P::Context, v: &mut dyn Visitor<P = P>) -> Result<(), P::Error> {
        Ok(())
    }
}
