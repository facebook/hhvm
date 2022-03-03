// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use super::{Error, Result, ShallowDeclCache, TypeDecl};
use crate::decl_defs::{ConstDecl, DeclTy, FunDecl};
use crate::decl_parser::DeclParser;
use crate::naming_provider::NamingProvider;
use crate::reason::Reason;
use pos::{ConstName, FunName, MethodName, PropName, RelativePath, TypeName};
use std::sync::Arc;

/// A `ShallowDeclProvider` which, if the requested name is not present in its
/// cache, uses the given naming table to find the file containing the requested
/// symbol, parses it with the given `DeclParser`, and inserts the parsed decls
/// into its cache.
#[derive(Debug)]
pub struct LazyShallowDeclProvider<R: Reason> {
    cache: Arc<ShallowDeclCache<R>>,
    naming_provider: Arc<dyn NamingProvider>,
    parser: DeclParser,
}

impl<R: Reason> LazyShallowDeclProvider<R> {
    pub fn new(
        cache: Arc<ShallowDeclCache<R>>,
        naming_provider: Arc<dyn NamingProvider>,
        parser: DeclParser,
    ) -> Self {
        Self {
            cache,
            naming_provider,
            parser,
        }
    }

    fn parse_and_cache_decls_in(&self, path: RelativePath) -> Result<()> {
        let decls_result = self.parser.parse(path);
        let decls = decls_result.map_err(|io_error| Error::DeclParse { path, io_error })?;
        self.cache.add_decls(decls);
        Ok(())
    }
}

impl<R: Reason> super::ShallowDeclProvider<R> for LazyShallowDeclProvider<R> {
    fn get_fun(&self, name: FunName) -> Result<Option<Arc<FunDecl<R>>>> {
        if let res @ Some(..) = self.cache.get_fun(name) {
            return Ok(res);
        }
        if let Some(path) = self.naming_provider.get_fun_path(name)? {
            self.parse_and_cache_decls_in(path)?;
            return Ok(self.cache.get_fun(name));
        }
        Ok(None)
    }

    fn get_const(&self, name: ConstName) -> Result<Option<Arc<ConstDecl<R>>>> {
        if let res @ Some(..) = self.cache.get_const(name) {
            return Ok(res);
        }
        if let Some(path) = self.naming_provider.get_const_path(name)? {
            self.parse_and_cache_decls_in(path)?;
            return Ok(self.cache.get_const(name));
        }
        Ok(None)
    }

    fn get_type(&self, name: TypeName) -> Result<Option<TypeDecl<R>>> {
        if let res @ Some(..) = self.cache.get_type(name) {
            return Ok(res);
        }
        if let Some(path) = self.naming_provider.get_type_path(name)? {
            self.parse_and_cache_decls_in(path)?;
            return Ok(self.cache.get_type(name));
        }
        Ok(None)
    }

    fn get_property_type(
        &self,
        class_name: TypeName,
        property_name: PropName,
    ) -> Result<Option<DeclTy<R>>> {
        if let res @ Some(..) = self.cache.get_property_type(class_name, property_name) {
            return Ok(res);
        }
        if let Some(path) = self.naming_provider.get_type_path(class_name)? {
            self.parse_and_cache_decls_in(path)?;
            return Ok(self.cache.get_property_type(class_name, property_name));
        }
        Ok(None)
    }

    fn get_static_property_type(
        &self,
        class_name: TypeName,
        property_name: PropName,
    ) -> Result<Option<DeclTy<R>>> {
        if let res @ Some(..) = self
            .cache
            .get_static_property_type(class_name, property_name)
        {
            return Ok(res);
        }
        if let Some(path) = self.naming_provider.get_type_path(class_name)? {
            self.parse_and_cache_decls_in(path)?;
            return Ok(self
                .cache
                .get_static_property_type(class_name, property_name));
        }
        Ok(None)
    }

    fn get_method_type(
        &self,
        class_name: TypeName,
        method_name: MethodName,
    ) -> Result<Option<DeclTy<R>>> {
        if let res @ Some(..) = self.cache.get_method_type(class_name, method_name) {
            return Ok(res);
        }
        if let Some(path) = self.naming_provider.get_type_path(class_name)? {
            self.parse_and_cache_decls_in(path)?;
            return Ok(self.cache.get_method_type(class_name, method_name));
        }
        Ok(None)
    }

    fn get_static_method_type(
        &self,
        class_name: TypeName,
        method_name: MethodName,
    ) -> Result<Option<DeclTy<R>>> {
        if let res @ Some(..) = self.cache.get_static_method_type(class_name, method_name) {
            return Ok(res);
        }
        if let Some(path) = self.naming_provider.get_type_path(class_name)? {
            self.parse_and_cache_decls_in(path)?;
            return Ok(self.cache.get_static_method_type(class_name, method_name));
        }
        Ok(None)
    }

    fn get_constructor_type(&self, class_name: TypeName) -> Result<Option<DeclTy<R>>> {
        if let res @ Some(..) = self.cache.get_constructor_type(class_name) {
            return Ok(res);
        }
        if let Some(path) = self.naming_provider.get_type_path(class_name)? {
            self.parse_and_cache_decls_in(path)?;
            return Ok(self.cache.get_constructor_type(class_name));
        }
        Ok(None)
    }
}

/// A `ShallowDeclProvider` which assumes its cache never evicts values and is
/// fully populated with all shallow decls in the repository (i.e., the cache
/// must be eagerly populated in advance).
#[derive(Debug)]
pub struct EagerShallowDeclProvider<R: Reason> {
    cache: Arc<ShallowDeclCache<R>>,
}

impl<R: Reason> EagerShallowDeclProvider<R> {
    pub fn new(cache: Arc<ShallowDeclCache<R>>) -> Self {
        Self { cache }
    }
}

impl<R: Reason> super::ShallowDeclProvider<R> for EagerShallowDeclProvider<R> {
    fn get_fun(&self, name: FunName) -> Result<Option<Arc<FunDecl<R>>>> {
        Ok(self.cache.get_fun(name))
    }

    fn get_const(&self, name: ConstName) -> Result<Option<Arc<ConstDecl<R>>>> {
        Ok(self.cache.get_const(name))
    }

    fn get_type(&self, name: TypeName) -> Result<Option<TypeDecl<R>>> {
        Ok(self.cache.get_type(name))
    }

    fn get_property_type(
        &self,
        class_name: TypeName,
        property_name: PropName,
    ) -> Result<Option<DeclTy<R>>> {
        Ok(self.cache.get_property_type(class_name, property_name))
    }

    fn get_static_property_type(
        &self,
        class_name: TypeName,
        property_name: PropName,
    ) -> Result<Option<DeclTy<R>>> {
        Ok(self
            .cache
            .get_static_property_type(class_name, property_name))
    }

    fn get_method_type(
        &self,
        class_name: TypeName,
        method_name: MethodName,
    ) -> Result<Option<DeclTy<R>>> {
        Ok(self.cache.get_method_type(class_name, method_name))
    }

    fn get_static_method_type(
        &self,
        class_name: TypeName,
        method_name: MethodName,
    ) -> Result<Option<DeclTy<R>>> {
        Ok(self.cache.get_static_method_type(class_name, method_name))
    }

    fn get_constructor_type(&self, class_name: TypeName) -> Result<Option<DeclTy<R>>> {
        Ok(self.cache.get_constructor_type(class_name))
    }
}
