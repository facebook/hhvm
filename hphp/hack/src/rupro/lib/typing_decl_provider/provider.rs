// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use super::{Class, ClassType, TypeDecl, TypingDeclProvider};
use crate::cache::Cache;
use crate::folded_decl_provider::FoldedDeclProvider;
use crate::reason::Reason;
use pos::TypeName;
use std::sync::Arc;

/// An implementation of TypingDeclProvider designed after the hh_server
/// implementation. We take the decls from a `FoldedDeclProvider` (which is
/// assumed to provide fast access to member types) and map them to a
/// representation which includes type signatures directly on class member
/// structs. This is done to reduce copying and overfetching for
/// `FoldedDeclProvider` implementations which store data in a serialized
/// format: we want to avoid copying the type of a method which is inherited by
/// 1000 classes into 1000 separate serialized blobs.
#[derive(Debug)]
pub struct FoldingTypingDeclProvider<R: Reason> {
    cache: Arc<dyn Cache<TypeName, Arc<ClassType<R>>>>,
    folded_decl_provider: Arc<dyn FoldedDeclProvider<R>>,
}

impl<R: Reason> FoldingTypingDeclProvider<R> {
    pub fn new(
        cache: Arc<dyn Cache<TypeName, Arc<ClassType<R>>>>,
        folded_decl_provider: Arc<dyn FoldedDeclProvider<R>>,
    ) -> Self {
        Self {
            cache,
            folded_decl_provider,
        }
    }
}

impl<R: Reason> TypingDeclProvider<R> for FoldingTypingDeclProvider<R> {
    fn get_class(&self, name: TypeName) -> Option<Arc<dyn Class<R>>> {
        match self.cache.get(name) {
            Some(rc) => Some(rc),
            None => {
                let folded_decl = self.folded_decl_provider.get_class(name)?;
                let cls = Arc::new(ClassType::new(
                    Arc::clone(&self.folded_decl_provider),
                    folded_decl,
                ));
                self.cache.insert(name, Arc::clone(&cls));
                Some(cls)
            }
        }
    }

    fn get_class_or_typedef(&self, name: TypeName) -> Option<TypeDecl<R>> {
        self.get_class(name).map(TypeDecl::Class)
    }
}
