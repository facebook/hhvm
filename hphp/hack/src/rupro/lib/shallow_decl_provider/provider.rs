// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use super::ShallowDeclCache;
use crate::decl_defs::{DeclTy, ShallowClass};
use crate::decl_parser::DeclParser;
use crate::reason::Reason;
use pos::{MethodName, PropName, TypeName};
use std::sync::Arc;

/// A `ShallowDeclProvider` which, if the requested name is not present in its
/// cache, uses the given naming table to find the file containing the requested
/// symbol, parses it with the given `DeclParser`, and inserts the parsed decls
/// into its cache.
#[derive(Debug)]
pub struct LazyShallowDeclProvider<R: Reason> {
    cache: Arc<ShallowDeclCache<R>>,
    #[allow(unused)] // will be used when lazy decl is implemented
    naming_table: (),
    #[allow(unused)] // will be used when lazy decl is implemented
    parser: DeclParser<R>,
}

impl<R: Reason> LazyShallowDeclProvider<R> {
    pub fn new(cache: Arc<ShallowDeclCache<R>>, naming_table: (), parser: DeclParser<R>) -> Self {
        Self {
            cache,
            naming_table,
            parser,
        }
    }
}

impl<R: Reason> super::ShallowDeclProvider<R> for LazyShallowDeclProvider<R> {
    fn get_class(&self, name: TypeName) -> Option<Arc<ShallowClass<R>>> {
        let res = self.cache.get_class(name);
        if res.is_some() { res } else { todo!() }
    }

    fn get_property_type(
        &self,
        class_name: TypeName,
        property_name: PropName,
    ) -> Option<DeclTy<R>> {
        let res = self.cache.get_property_type(class_name, property_name);
        if res.is_some() { res } else { todo!() }
    }

    fn get_static_property_type(
        &self,
        class_name: TypeName,
        property_name: PropName,
    ) -> Option<DeclTy<R>> {
        let res = self
            .cache
            .get_static_property_type(class_name, property_name);
        if res.is_some() { res } else { todo!() }
    }

    fn get_method_type(&self, class_name: TypeName, method_name: MethodName) -> Option<DeclTy<R>> {
        let res = self.cache.get_method_type(class_name, method_name);
        if res.is_some() { res } else { todo!() }
    }

    fn get_static_method_type(
        &self,
        class_name: TypeName,
        method_name: MethodName,
    ) -> Option<DeclTy<R>> {
        let res = self.cache.get_static_method_type(class_name, method_name);
        if res.is_some() { res } else { todo!() }
    }

    fn get_constructor_type(&self, class_name: TypeName) -> Option<DeclTy<R>> {
        let res = self.cache.get_constructor_type(class_name);
        if res.is_some() { res } else { todo!() }
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
