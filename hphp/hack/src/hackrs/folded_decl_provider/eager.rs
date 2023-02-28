// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::Arc;

use datastore::Store;
use oxidized::naming_types::KindOfType;
use pos::ConstName;
use pos::FunName;
use pos::ModuleName;
use pos::TypeName;
use shallow_decl_provider::ShallowDeclProvider;
use ty::decl::ConstDecl;
use ty::decl::FoldedClass;
use ty::decl::FunDecl;
use ty::decl::ModuleDecl;
use ty::reason::Reason;

use super::Error;
use super::Result;
use super::TypeDecl;

/// A `FoldedDeclProvider` which assumes that all extant decls have eagerly been
/// inserted in its store. It returns `None` when asked for a decl which is not
/// present in the store.
#[derive(Debug)]
pub struct EagerFoldedDeclProvider<R: Reason> {
    store: Arc<dyn Store<TypeName, Arc<FoldedClass<R>>>>,
    shallow_decl_provider: Arc<dyn ShallowDeclProvider<R>>,
}

impl<R: Reason> EagerFoldedDeclProvider<R> {
    pub fn new(
        store: Arc<dyn Store<TypeName, Arc<FoldedClass<R>>>>,
        shallow_decl_provider: Arc<dyn ShallowDeclProvider<R>>,
    ) -> Self {
        Self {
            store,
            shallow_decl_provider,
        }
    }
}

impl<R: Reason> super::FoldedDeclProvider<R> for EagerFoldedDeclProvider<R> {
    fn get_fun(&self, name: FunName) -> Result<Option<Arc<FunDecl<R>>>> {
        Ok(self.shallow_decl_provider.get_fun(name)?)
    }

    fn get_const(&self, name: ConstName) -> Result<Option<Arc<ConstDecl<R>>>> {
        Ok(self.shallow_decl_provider.get_const(name)?)
    }

    fn get_module(&self, name: ModuleName) -> Result<Option<Arc<ModuleDecl<R>>>> {
        Ok(self.shallow_decl_provider.get_module(name)?)
    }

    fn get_type(&self, name: TypeName) -> Result<Option<TypeDecl<R>>> {
        match self.shallow_decl_provider.get_type_kind(name)? {
            None => Ok(None),
            Some(KindOfType::TTypedef) => Ok(Some(TypeDecl::Typedef(
                self.shallow_decl_provider.get_typedef(name)?.expect(
                    "got None after get_type_kind indicated a typedef with this name exists",
                ),
            ))),
            Some(KindOfType::TClass) => match self.store.get(name).map_err(Error::Store)? {
                Some(c) => Ok(Some(TypeDecl::Class(c))),
                None => Ok(None),
            },
        }
    }
}
