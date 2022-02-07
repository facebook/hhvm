// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::decl_defs::ShallowClass;
use crate::reason::Reason;
use crate::shallow_decl_provider::ShallowDeclCache;
use pos::{RelativePathCtx, TypeName};
use std::sync::Arc;

#[derive(Debug)]
pub struct ShallowDeclProvider<R: Reason> {
    cache: Arc<ShallowDeclCache<R>>,
    #[allow(unused)] // will be used when lazy decl is implemented
    relative_path_ctx: Arc<RelativePathCtx>,
}

impl<R: Reason> ShallowDeclProvider<R> {
    pub fn new(cache: Arc<ShallowDeclCache<R>>, relative_path_ctx: Arc<RelativePathCtx>) -> Self {
        Self {
            cache,
            relative_path_ctx,
        }
    }

    pub fn get_shallow_class(&self, name: TypeName) -> Option<Arc<ShallowClass<R>>> {
        self.cache.classes.get(name)
    }
}
