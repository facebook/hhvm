// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<58b0869f8a55e329f7c79a4bac463093>>
//
// To regenerate this file, run:
//   buck run @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:oxidized_regen

use super::Visitor;
pub trait Node<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        self.recurse(v)
    }
    fn recurse(&'a self, _v: &mut dyn Visitor<'a>) {}
}
