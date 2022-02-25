// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use super::{ShallowDeclCache, TypeDecl};
use crate::decl_defs::{ConstDecl, DeclTy, FunDecl};
use crate::decl_parser::DeclParser;
use crate::naming_provider::NamingProvider;
use crate::reason::Reason;
use pos::{ConstName, FunName, MethodName, PropName, RelativePath, TypeName};
use std::io;
use std::sync::Arc;

/// A `ShallowDeclProvider` which, if the requested name is not present in its
/// cache, uses the given naming table to find the file containing the requested
/// symbol, parses it with the given `DeclParser`, and inserts the parsed decls
/// into its cache.
#[derive(Debug)]
pub struct LazyShallowDeclProvider<R: Reason> {
    cache: Arc<ShallowDeclCache<R>>,
    naming_provider: Arc<dyn NamingProvider>,
    parser: DeclParser<R>,
}

impl<R: Reason> LazyShallowDeclProvider<R> {
    pub fn new(
        cache: Arc<ShallowDeclCache<R>>,
        naming_provider: Arc<dyn NamingProvider>,
        parser: DeclParser<R>,
    ) -> Self {
        Self {
            cache,
            naming_provider,
            parser,
        }
    }

    fn parse_and_cache_decls_in(&self, path: RelativePath) -> io::Result<()> {
        self.cache.add_decls(self.parser.parse(path)?);
        Ok(())
    }
}

impl<R: Reason> super::ShallowDeclProvider<R> for LazyShallowDeclProvider<R> {
    fn get_fun(&self, name: FunName) -> Option<Arc<FunDecl<R>>> {
        if let res @ Some(..) = self.cache.get_fun(name) {
            return res;
        }
        if let Some(path) = self.naming_provider.get_fun_path(name).unwrap() {
            self.parse_and_cache_decls_in(path).unwrap();
            return self.cache.get_fun(name);
        }
        None
    }

    fn get_const(&self, name: ConstName) -> Option<Arc<ConstDecl<R>>> {
        if let res @ Some(..) = self.cache.get_const(name) {
            return res;
        }
        if let Some(path) = self.naming_provider.get_const_path(name).unwrap() {
            self.parse_and_cache_decls_in(path).unwrap();
            return self.cache.get_const(name);
        }
        None
    }

    fn get_type(&self, name: TypeName) -> Option<TypeDecl<R>> {
        if let res @ Some(..) = self.cache.get_type(name) {
            return res;
        }
        if let Some(path) = self.naming_provider.get_type_path(name).unwrap() {
            self.parse_and_cache_decls_in(path).unwrap();
            return self.cache.get_type(name);
        }
        None
    }

    fn get_property_type(
        &self,
        class_name: TypeName,
        property_name: PropName,
    ) -> Option<DeclTy<R>> {
        if let res @ Some(..) = self.cache.get_property_type(class_name, property_name) {
            return res;
        }
        if let Some(path) = self.naming_provider.get_type_path(class_name).unwrap() {
            self.parse_and_cache_decls_in(path).unwrap();
            return self.cache.get_property_type(class_name, property_name);
        }
        None
    }

    fn get_static_property_type(
        &self,
        class_name: TypeName,
        property_name: PropName,
    ) -> Option<DeclTy<R>> {
        if let res @ Some(..) = self
            .cache
            .get_static_property_type(class_name, property_name)
        {
            return res;
        }
        if let Some(path) = self.naming_provider.get_type_path(class_name).unwrap() {
            self.parse_and_cache_decls_in(path).unwrap();
            return self
                .cache
                .get_static_property_type(class_name, property_name);
        }
        None
    }

    fn get_method_type(&self, class_name: TypeName, method_name: MethodName) -> Option<DeclTy<R>> {
        if let res @ Some(..) = self.cache.get_method_type(class_name, method_name) {
            return res;
        }
        if let Some(path) = self.naming_provider.get_type_path(class_name).unwrap() {
            self.parse_and_cache_decls_in(path).unwrap();
            return self.cache.get_method_type(class_name, method_name);
        }
        None
    }

    fn get_static_method_type(
        &self,
        class_name: TypeName,
        method_name: MethodName,
    ) -> Option<DeclTy<R>> {
        if let res @ Some(..) = self.cache.get_static_method_type(class_name, method_name) {
            return res;
        }
        if let Some(path) = self.naming_provider.get_type_path(class_name).unwrap() {
            self.parse_and_cache_decls_in(path).unwrap();
            return self.cache.get_static_method_type(class_name, method_name);
        }
        None
    }

    fn get_constructor_type(&self, class_name: TypeName) -> Option<DeclTy<R>> {
        if let res @ Some(..) = self.cache.get_constructor_type(class_name) {
            return res;
        }
        if let Some(path) = self.naming_provider.get_type_path(class_name).unwrap() {
            self.parse_and_cache_decls_in(path).unwrap();
            return self.cache.get_constructor_type(class_name);
        }
        None
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
    fn get_fun(&self, name: FunName) -> Option<Arc<FunDecl<R>>> {
        self.cache.get_fun(name)
    }

    fn get_const(&self, name: ConstName) -> Option<Arc<ConstDecl<R>>> {
        self.cache.get_const(name)
    }

    fn get_type(&self, name: TypeName) -> Option<TypeDecl<R>> {
        self.cache.get_type(name)
    }

    fn get_property_type(
        &self,
        class_name: TypeName,
        property_name: PropName,
    ) -> Option<DeclTy<R>> {
        self.cache.get_property_type(class_name, property_name)
    }

    fn get_static_property_type(
        &self,
        class_name: TypeName,
        property_name: PropName,
    ) -> Option<DeclTy<R>> {
        self.cache
            .get_static_property_type(class_name, property_name)
    }

    fn get_method_type(&self, class_name: TypeName, method_name: MethodName) -> Option<DeclTy<R>> {
        self.cache.get_method_type(class_name, method_name)
    }

    fn get_static_method_type(
        &self,
        class_name: TypeName,
        method_name: MethodName,
    ) -> Option<DeclTy<R>> {
        self.cache.get_static_method_type(class_name, method_name)
    }

    fn get_constructor_type(&self, class_name: TypeName) -> Option<DeclTy<R>> {
        self.cache.get_constructor_type(class_name)
    }
}
