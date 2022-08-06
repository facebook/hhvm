// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::Arc;

use decl_parser::DeclParser;
use itertools::Itertools;
use naming_provider::NamingProvider;
use oxidized::naming_types::KindOfType;
use pos::ConstName;
use pos::FunName;
use pos::MethodName;
use pos::PropName;
use pos::RelativePath;
use pos::TypeName;
use ty::decl::shallow::Decl;
use ty::decl::ConstDecl;
use ty::decl::FunDecl;
use ty::decl::ShallowClass;
use ty::decl::Ty;
use ty::decl::TypedefDecl;
use ty::reason::Reason;

use super::Error;
use super::Result;
use super::ShallowDeclStore;
use super::TypeDecl;

/// A `ShallowDeclProvider` which, if the requested name is not present in its
/// store, uses the given naming table to find the file containing the requested
/// symbol, parses it with the given `DeclParser`, and inserts the parsed decls
/// into its store.
#[derive(Debug)]
pub struct LazyShallowDeclProvider<R: Reason> {
    store: Arc<ShallowDeclStore<R>>,
    naming_provider: Arc<dyn NamingProvider>,
    parser: DeclParser<R>,
}

impl<R: Reason> LazyShallowDeclProvider<R> {
    pub fn new(
        store: Arc<ShallowDeclStore<R>>,
        naming_provider: Arc<dyn NamingProvider>,
        parser: DeclParser<R>,
    ) -> Self {
        Self {
            store,
            naming_provider,
            parser,
        }
    }

    pub fn parse_and_cache_decls_in(&self, path: RelativePath) -> Result<()> {
        let decls_result = self.parser.parse(path);
        let decls = decls_result.map_err(|file_provider_error| Error::DeclParse {
            path,
            file_provider_error,
        })?;
        self.dedup_and_add_decls(path, decls)?;
        Ok(())
    }

    pub fn dedup_and_add_decls(
        &self,
        path: RelativePath,
        decls: impl IntoIterator<Item = Decl<R>>,
    ) -> Result<()> {
        // dedup, taking the decl which was declared first syntactically
        let decls = decls
            .into_iter()
            .unique_by(|decl| (decl.name(), decl.name_kind()));
        // dedup with symbols declared in other files
        let decls = self.remove_naming_conflict_losers(path, decls)?;
        self.store.add_decls(decls)?;
        Ok(())
    }

    /// If a symbol was also declared in another file, and that file
    /// was determined to be the winner in the naming table, remove
    /// its decl from the list.
    fn remove_naming_conflict_losers(
        &self,
        path: RelativePath,
        decls: impl Iterator<Item = Decl<R>>,
    ) -> Result<Vec<Decl<R>>> {
        let mut winners = vec![];
        for decl in decls {
            let path_opt = match decl {
                Decl::Class(name, _) | Decl::Typedef(name, _) => {
                    self.naming_provider.get_type_path(name)?
                }
                Decl::Fun(name, _) => self.naming_provider.get_fun_path(name)?,
                Decl::Const(name, _) => self.naming_provider.get_const_path(name)?,
                Decl::Module(..) => Some(path), // TODO: look this up from naming provider
            };
            if path_opt.map_or(true, |p| p == path) {
                winners.push(decl)
            }
        }
        Ok(winners)
    }
}

impl<R: Reason> super::ShallowDeclProvider<R> for LazyShallowDeclProvider<R> {
    fn get_fun(&self, name: FunName) -> Result<Option<Arc<FunDecl<R>>>> {
        if let res @ Some(..) = self.store.get_fun(name)? {
            return Ok(res);
        }
        if let Some(path) = self.naming_provider.get_fun_path(name)? {
            self.parse_and_cache_decls_in(path)?;
            return Ok(self.store.get_fun(name)?);
        }
        Ok(None)
    }

    fn get_const(&self, name: ConstName) -> Result<Option<Arc<ConstDecl<R>>>> {
        if let res @ Some(..) = self.store.get_const(name)? {
            return Ok(res);
        }
        if let Some(path) = self.naming_provider.get_const_path(name)? {
            self.parse_and_cache_decls_in(path)?;
            return Ok(self.store.get_const(name)?);
        }
        Ok(None)
    }

    fn get_type_kind(&self, name: TypeName) -> Result<Option<KindOfType>> {
        Ok(self
            .naming_provider
            .get_type_path_and_kind(name)?
            .map(|(_path, kind)| kind))
    }

    fn get_type(&self, name: TypeName) -> Result<Option<TypeDecl<R>>> {
        if let Some(kind) = self.get_type_kind(name)? {
            match kind {
                KindOfType::TClass => Ok(self.get_class(name)?.map(|decl| TypeDecl::Class(decl))),
                KindOfType::TTypedef => {
                    Ok(self.get_typedef(name)?.map(|decl| TypeDecl::Typedef(decl)))
                }
            }
        } else {
            Ok(None)
        }
    }

    fn get_typedef(&self, name: TypeName) -> Result<Option<Arc<TypedefDecl<R>>>> {
        if let res @ Some(..) = self.store.get_typedef(name)? {
            return Ok(res);
        }
        if let Some(path) = self.naming_provider.get_type_path(name)? {
            self.parse_and_cache_decls_in(path)?;
            return Ok(self.store.get_typedef(name)?);
        }
        Ok(None)
    }

    fn get_class(&self, name: TypeName) -> Result<Option<Arc<ShallowClass<R>>>> {
        if let res @ Some(..) = self.store.get_class(name)? {
            return Ok(res);
        }
        if let Some(path) = self.naming_provider.get_type_path(name)? {
            self.parse_and_cache_decls_in(path)?;
            return Ok(self.store.get_class(name)?);
        }
        Ok(None)
    }

    fn get_property_type(
        &self,
        class_name: TypeName,
        property_name: PropName,
    ) -> Result<Option<Ty<R>>> {
        if let res @ Some(..) = self.store.get_property_type(class_name, property_name)? {
            return Ok(res);
        }
        if let Some(path) = self.naming_provider.get_type_path(class_name)? {
            self.parse_and_cache_decls_in(path)?;
            return Ok(self.store.get_property_type(class_name, property_name)?);
        }
        Ok(None)
    }

    fn get_static_property_type(
        &self,
        class_name: TypeName,
        property_name: PropName,
    ) -> Result<Option<Ty<R>>> {
        if let res @ Some(..) = self
            .store
            .get_static_property_type(class_name, property_name)?
        {
            return Ok(res);
        }
        if let Some(path) = self.naming_provider.get_type_path(class_name)? {
            self.parse_and_cache_decls_in(path)?;
            return Ok(self
                .store
                .get_static_property_type(class_name, property_name)?);
        }
        Ok(None)
    }

    fn get_method_type(
        &self,
        class_name: TypeName,
        method_name: MethodName,
    ) -> Result<Option<Ty<R>>> {
        if let res @ Some(..) = self.store.get_method_type(class_name, method_name)? {
            return Ok(res);
        }
        if let Some(path) = self.naming_provider.get_type_path(class_name)? {
            self.parse_and_cache_decls_in(path)?;
            return Ok(self.store.get_method_type(class_name, method_name)?);
        }
        Ok(None)
    }

    fn get_static_method_type(
        &self,
        class_name: TypeName,
        method_name: MethodName,
    ) -> Result<Option<Ty<R>>> {
        if let res @ Some(..) = self.store.get_static_method_type(class_name, method_name)? {
            return Ok(res);
        }
        if let Some(path) = self.naming_provider.get_type_path(class_name)? {
            self.parse_and_cache_decls_in(path)?;
            return Ok(self.store.get_static_method_type(class_name, method_name)?);
        }
        Ok(None)
    }

    fn get_constructor_type(&self, class_name: TypeName) -> Result<Option<Ty<R>>> {
        if let res @ Some(..) = self.store.get_constructor_type(class_name)? {
            return Ok(res);
        }
        if let Some(path) = self.naming_provider.get_type_path(class_name)? {
            self.parse_and_cache_decls_in(path)?;
            return Ok(self.store.get_constructor_type(class_name)?);
        }
        Ok(None)
    }
}

/// A `ShallowDeclProvider` which assumes its store never evicts values and is
/// fully populated with all shallow decls in the repository (i.e., the store
/// must be eagerly populated in advance).
#[derive(Debug)]
pub struct EagerShallowDeclProvider<R: Reason> {
    store: Arc<ShallowDeclStore<R>>,
}

impl<R: Reason> EagerShallowDeclProvider<R> {
    pub fn new(store: Arc<ShallowDeclStore<R>>) -> Self {
        Self { store }
    }
}

impl<R: Reason> super::ShallowDeclProvider<R> for EagerShallowDeclProvider<R> {
    fn get_fun(&self, name: FunName) -> Result<Option<Arc<FunDecl<R>>>> {
        Ok(self.store.get_fun(name)?)
    }

    fn get_const(&self, name: ConstName) -> Result<Option<Arc<ConstDecl<R>>>> {
        Ok(self.store.get_const(name)?)
    }

    fn get_type_kind(&self, name: TypeName) -> Result<Option<KindOfType>> {
        if self.get_class(name)?.is_some() {
            Ok(Some(KindOfType::TClass))
        } else if self.get_typedef(name)?.is_some() {
            Ok(Some(KindOfType::TTypedef))
        } else {
            Ok(None)
        }
    }

    fn get_type(&self, name: TypeName) -> Result<Option<TypeDecl<R>>> {
        if let Some(class) = self.get_class(name)? {
            Ok(Some(TypeDecl::Class(class)))
        } else {
            Ok(self.get_typedef(name)?.map(TypeDecl::Typedef))
        }
    }

    fn get_typedef(&self, name: TypeName) -> Result<Option<Arc<TypedefDecl<R>>>> {
        Ok(self.store.get_typedef(name)?)
    }

    fn get_class(&self, name: TypeName) -> Result<Option<Arc<ShallowClass<R>>>> {
        Ok(self.store.get_class(name)?)
    }

    fn get_property_type(
        &self,
        class_name: TypeName,
        property_name: PropName,
    ) -> Result<Option<Ty<R>>> {
        Ok(self.store.get_property_type(class_name, property_name)?)
    }

    fn get_static_property_type(
        &self,
        class_name: TypeName,
        property_name: PropName,
    ) -> Result<Option<Ty<R>>> {
        Ok(self
            .store
            .get_static_property_type(class_name, property_name)?)
    }

    fn get_method_type(
        &self,
        class_name: TypeName,
        method_name: MethodName,
    ) -> Result<Option<Ty<R>>> {
        Ok(self.store.get_method_type(class_name, method_name)?)
    }

    fn get_static_method_type(
        &self,
        class_name: TypeName,
        method_name: MethodName,
    ) -> Result<Option<Ty<R>>> {
        Ok(self.store.get_static_method_type(class_name, method_name)?)
    }

    fn get_constructor_type(&self, class_name: TypeName) -> Result<Option<Ty<R>>> {
        Ok(self.store.get_constructor_type(class_name)?)
    }
}
