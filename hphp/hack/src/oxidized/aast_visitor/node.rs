// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<0453248c125b4001026779bd5746c538>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

#![allow(unused_variables)]
use super::visitor::Visitor;
pub trait Node<Context, Error, Ex, Fb, En, Hi> {
    fn accept(
        &self,
        ctx: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        self.recurse(ctx, v)
    }
    fn recurse(
        &self,
        ctx: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        Ok(())
    }
}
