// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::cache::Cache;
use crate::folded_decl_provider::FoldedDeclProvider;
use crate::reason::Reason;
use crate::typing_decl_provider::Class;
use pos::TypeName;
use std::sync::Arc;

#[derive(Debug)]
pub struct TypingDeclProvider<R: Reason> {
    cache: Arc<dyn Cache<TypeName, Arc<Class<R>>>>,
    folded_decl_provider: Arc<FoldedDeclProvider<R>>,
}

impl<R: Reason> TypingDeclProvider<R> {
    pub fn new(
        cache: Arc<dyn Cache<TypeName, Arc<Class<R>>>>,
        folded_decl_provider: Arc<FoldedDeclProvider<R>>,
    ) -> Self {
        Self {
            cache,
            folded_decl_provider,
        }
    }

    pub fn get_class(&self, name: TypeName) -> Option<Arc<Class<R>>> {
        match self.cache.get(name) {
            Some(rc) => Some(rc),
            None => {
                let folded_decl = self.folded_decl_provider.get_folded_class(name)?;
                let cls = Arc::new(Class::new(
                    Arc::clone(&self.folded_decl_provider),
                    folded_decl,
                ));
                self.cache.insert(name, Arc::clone(&cls));
                Some(cls)
            }
        }
    }

    pub fn get_class_or_typedef(&self, name: TypeName) -> Option<Arc<Class<R>>> {
        self.get_class(name)
    }
}
