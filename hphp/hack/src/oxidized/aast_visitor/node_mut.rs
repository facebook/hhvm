// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<1ba997789a7557c7e8b4a52b47a343fb>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

#![allow(unused_variables)]
use super::visitor_mut::VisitorMut;
pub trait NodeMut<Context, Error, Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        ctx: &mut Context,
        v: &mut dyn VisitorMut<
            Context = Context,
            Error = Error,
            Ex = Ex,
            Fb = Fb,
            En = En,
            Hi = Hi,
        >,
    ) -> Result<(), Error> {
        self.recurse(ctx, v)
    }
    fn recurse(
        &mut self,
        ctx: &mut Context,
        v: &mut dyn VisitorMut<
            Context = Context,
            Error = Error,
            Ex = Ex,
            Fb = Fb,
            En = En,
            Hi = Hi,
        >,
    ) -> Result<(), Error> {
        Ok(())
    }
}
