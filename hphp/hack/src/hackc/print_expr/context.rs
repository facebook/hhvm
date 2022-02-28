// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::special_class_resolver::SpecialClassResolver;
use env::emitter::Emitter;

#[derive(Clone)]
pub struct Context<'a> {
    pub(crate) special_class_resolver: &'a dyn SpecialClassResolver,
    pub(crate) dump_lambdas: bool,
}

impl<'a> Context<'a> {
    pub fn new<'arena, 'decl>(emitter: &'a Emitter<'arena, 'decl>) -> Self {
        Self {
            special_class_resolver: emitter,
            dump_lambdas: false,
        }
    }
}
