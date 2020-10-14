// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use super::Visitor;

pub trait Node<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        self.recurse(v)
    }
    fn recurse(&'a self, _v: &mut dyn Visitor<'a>) {}
}
