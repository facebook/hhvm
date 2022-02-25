// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use super::{ClassType, TypeDecl, TypingDeclProvider};
use crate::cache::Cache;
use crate::decl_defs::{ConstDecl, FunDecl};
use crate::folded_decl_provider::{self, FoldedDeclProvider};
use crate::reason::Reason;
use pos::{ConstName, FunName, TypeName};
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
    fn get_fun(&self, name: FunName) -> Option<Arc<FunDecl<R>>> {
        self.folded_decl_provider.get_fun(name).unwrap()
    }

    fn get_const(&self, name: ConstName) -> Option<Arc<ConstDecl<R>>> {
        self.folded_decl_provider.get_const(name).unwrap()
    }

    fn get_type(&self, name: TypeName) -> Option<TypeDecl<R>> {
        match self.cache.get(name) {
            Some(arc) => Some(TypeDecl::Class(arc)),
            None => match self.folded_decl_provider.get_type(name).unwrap()? {
                folded_decl_provider::TypeDecl::Typedef(decl) => Some(TypeDecl::Typedef(decl)),
                folded_decl_provider::TypeDecl::Class(folded_decl) => {
                    let cls = Arc::new(ClassType::new(
                        Arc::clone(&self.folded_decl_provider),
                        folded_decl,
                    ));
                    self.cache.insert(name, Arc::clone(&cls));
                    Some(TypeDecl::Class(cls))
                }
            },
        }
    }
}
