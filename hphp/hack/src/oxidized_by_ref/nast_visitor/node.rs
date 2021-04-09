// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<9544a146f80992ea1285b9e48a4eb19e>>
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
