// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<3a75b3a4d0be8c85ac53ef279f68c594>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use super::Visitor;
pub trait Node<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        self.recurse(v)
    }
    fn recurse(&'a self, _v: &mut dyn Visitor<'a>) {}
}
