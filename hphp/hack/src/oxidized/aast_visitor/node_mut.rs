// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<ababf5045874803605c5ef62e63a9e32>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

#![allow(unused_variables)]
use super::visitor_mut::VisitorMut;
pub trait NodeMut<Context, Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        ctx: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        self.recurse(ctx, v)
    }
    fn recurse(
        &mut self,
        ctx: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
    }
}
