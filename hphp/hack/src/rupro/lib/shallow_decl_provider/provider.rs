// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use super::ShallowDeclCache;
use crate::decl_defs::{DeclTy, ShallowClass};
use crate::decl_parser::DeclParser;
use crate::naming_provider::NamingProvider;
use crate::reason::Reason;
use pos::{MethodName, PropName, RelativePath, TypeName};
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
    fn get_class(&self, name: TypeName) -> Option<Arc<ShallowClass<R>>> {
        if let res @ Some(..) = self.cache.get_class(name) {
            return res;
        }
        if let Some(path) = self.naming_provider.get_type_path(name) {
            self.parse_and_cache_decls_in(path).unwrap();
            return self.cache.get_class(name);
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
        if let Some(path) = self.naming_provider.get_type_path(class_name) {
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
        if let Some(path) = self.naming_provider.get_type_path(class_name) {
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
        if let Some(path) = self.naming_provider.get_type_path(class_name) {
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
        if let Some(path) = self.naming_provider.get_type_path(class_name) {
            self.parse_and_cache_decls_in(path).unwrap();
            return self.cache.get_static_method_type(class_name, method_name);
        }
        None
    }

    fn get_constructor_type(&self, class_name: TypeName) -> Option<DeclTy<R>> {
        if let res @ Some(..) = self.cache.get_constructor_type(class_name) {
            return res;
        }
        if let Some(path) = self.naming_provider.get_type_path(class_name) {
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
    fn get_class(&self, name: TypeName) -> Option<Arc<ShallowClass<R>>> {
        self.cache.get_class(name)
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
