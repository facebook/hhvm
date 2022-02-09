// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::decl_defs::{DeclTy, ShallowClass};
use crate::reason::Reason;
use crate::shallow_decl_provider::ShallowDeclCache;
use pos::{MethodName, PropName, RelativePathCtx, TypeName};
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
        self.cache.get_class(name)
    }

    pub fn get_property_type(
        &self,
        class_name: TypeName,
        property_name: PropName,
    ) -> Option<DeclTy<R>> {
        self.cache.get_property_type(class_name, property_name)
    }

    pub fn get_static_property_type(
        &self,
        class_name: TypeName,
        property_name: PropName,
    ) -> Option<DeclTy<R>> {
        self.cache
            .get_static_property_type(class_name, property_name)
    }

    pub fn get_method_type(
        &self,
        class_name: TypeName,
        method_name: MethodName,
    ) -> Option<DeclTy<R>> {
        self.cache.get_method_type(class_name, method_name)
    }

    pub fn get_static_method_type(
        &self,
        class_name: TypeName,
        method_name: MethodName,
    ) -> Option<DeclTy<R>> {
        self.cache.get_static_method_type(class_name, method_name)
    }

    pub fn get_constructor_type(&self, class_name: TypeName) -> Option<DeclTy<R>> {
        self.cache.get_constructor_type(class_name)
    }
}
