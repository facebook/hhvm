// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<387c9adc078058e14d79e9f1e71c7788>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

#![allow(unused_variables)]
use super::type_params::Params;
use super::visitor_mut::VisitorMut;
pub trait NodeMut<P: Params> {
    fn accept(
        &mut self,
        ctx: &mut P::Context,
        v: &mut dyn VisitorMut<P = P>,
    ) -> Result<(), P::Error> {
        self.recurse(ctx, v)
    }
    fn recurse(
        &mut self,
        ctx: &mut P::Context,
        v: &mut dyn VisitorMut<P = P>,
    ) -> Result<(), P::Error> {
        Ok(())
    }
}
