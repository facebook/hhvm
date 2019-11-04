// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<efb066cfe91610c98e907edf7fe5be1b>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

#![allow(unused_variables)]
use super::visitor::Visitor;
pub trait Node<Context, Ex, Fb, En, Hi> {
    fn accept(
        &self,
        ctx: &mut Context,
        v: &mut dyn Visitor<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        self.recurse(ctx, v)
    }
    fn recurse(
        &self,
        ctx: &mut Context,
        v: &mut dyn Visitor<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
    }
}
