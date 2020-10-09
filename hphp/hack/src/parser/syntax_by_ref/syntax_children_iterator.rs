// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use super::{syntax::*, syntax_variant_generated::SyntaxVariant};
pub struct SyntaxChildrenIterator<'a, T, V> {
    pub syntax: &'a SyntaxVariant<'a, T, V>,
    pub index: usize,
    pub index_back: usize,
}

impl<'a, T, V> Iterator for SyntaxChildrenIterator<'a, T, V> {
    type Item = &'a Syntax<'a, T, V>;
    fn next(&mut self) -> Option<Self::Item> {
        self.next_impl(true)
    }
}
