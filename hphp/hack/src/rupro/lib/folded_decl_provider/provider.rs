// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use super::fold::DeclFolder;
use crate::alloc::Allocator;
use crate::cache::Cache;
use crate::decl_defs::{DeclTy, DeclTy_, FoldedClass, ShallowClass};
use crate::reason::Reason;
use crate::shallow_decl_provider::ShallowDeclProvider;
use crate::special_names::SpecialNames;
use pos::{MethodName, Positioned, PropName, TypeName, TypeNameMap, TypeNameSet};
use std::sync::Arc;

// note(sf, 2022-02-03): c.f. hphp/hack/src/decl/decl_folded_class.ml

/// A `FoldedDeclProvider` which, if the requested class name is not present in
/// its cache, recursively computes the folded decl for that class by requesting
/// the shallow decls of that class and its ancestors from its
/// `ShallowDeclProvider`.
#[derive(Debug)]
pub struct LazyFoldedDeclProvider<R: Reason> {
    cache: Arc<dyn Cache<TypeName, Arc<FoldedClass<R>>>>,
    alloc: &'static Allocator<R>,
    special_names: &'static SpecialNames,
    shallow_decl_provider: Arc<dyn ShallowDeclProvider<R>>,
}

impl<R: Reason> LazyFoldedDeclProvider<R> {
    pub fn new(
        cache: Arc<dyn Cache<TypeName, Arc<FoldedClass<R>>>>,
        alloc: &'static Allocator<R>,
        special_names: &'static SpecialNames,
        shallow_decl_provider: Arc<dyn ShallowDeclProvider<R>>,
    ) -> Self {
        Self {
            cache,
            alloc,
            special_names,
            shallow_decl_provider,
        }
    }
}

impl<R: Reason> super::FoldedDeclProvider<R> for LazyFoldedDeclProvider<R> {
    fn get_class(&self, name: TypeName) -> Option<Arc<FoldedClass<R>>> {
        let mut stack = Default::default();
        self.get_folded_class_impl(&mut stack, name)
    }

    fn get_shallow_property_type(
        &self,
        class_name: TypeName,
        property_name: PropName,
    ) -> Option<DeclTy<R>> {
        self.shallow_decl_provider
            .get_property_type(class_name, property_name)
    }

    fn get_shallow_static_property_type(
        &self,
        class_name: TypeName,
        property_name: PropName,
    ) -> Option<DeclTy<R>> {
        self.shallow_decl_provider
            .get_static_property_type(class_name, property_name)
    }

    fn get_shallow_method_type(
        &self,
        class_name: TypeName,
        method_name: MethodName,
    ) -> Option<DeclTy<R>> {
        self.shallow_decl_provider
            .get_method_type(class_name, method_name)
    }

    fn get_shallow_static_method_type(
        &self,
        class_name: TypeName,
        method_name: MethodName,
    ) -> Option<DeclTy<R>> {
        self.shallow_decl_provider
            .get_static_method_type(class_name, method_name)
    }

    fn get_shallow_constructor_type(&self, class_name: TypeName) -> Option<DeclTy<R>> {
        self.shallow_decl_provider.get_constructor_type(class_name)
    }
}

impl<R: Reason> LazyFoldedDeclProvider<R> {
    fn detect_cycle(&self, stack: &mut TypeNameSet, pos_id: &Positioned<TypeName, R::Pos>) -> bool {
        if stack.contains(&pos_id.id()) {
            todo!("TODO(hrust): register error");
        }
        false
    }

    fn decl_class_type(
        &self,
        stack: &mut TypeNameSet,
        acc: &mut TypeNameMap<Arc<FoldedClass<R>>>,
        ty: &DeclTy<R>,
    ) {
        match &**ty.node() {
            DeclTy_::DTapply(id_and_args) => {
                let pos_id = &id_and_args.0;
                if !self.detect_cycle(stack, pos_id) {
                    if let Some(folded_decl) = self.get_folded_class_impl(stack, pos_id.id()) {
                        acc.insert(pos_id.id(), folded_decl);
                    }
                }
            }
            _ => {}
        }
    }

    fn decl_class_parents(
        &self,
        stack: &mut TypeNameSet,
        sc: &ShallowClass<R>,
    ) -> TypeNameMap<Arc<FoldedClass<R>>> {
        let mut acc = Default::default();
        sc.extends
            .iter()
            .for_each(|ty| self.decl_class_type(stack, &mut acc, ty));
        acc
    }

    fn decl_class(&self, stack: &mut TypeNameSet, name: TypeName) -> Option<Arc<FoldedClass<R>>> {
        let shallow_class = self.shallow_decl_provider.get_class(name)?;
        stack.insert(name);
        let parents = self.decl_class_parents(stack, &shallow_class);
        let folder = DeclFolder::new(self.alloc, self.special_names);
        Some(folder.decl_class(&shallow_class, &parents))
    }

    fn get_folded_class_impl(
        &self,
        stack: &mut TypeNameSet,
        name: TypeName,
    ) -> Option<Arc<FoldedClass<R>>> {
        match self.cache.get(name) {
            Some(rc) => Some(rc),
            None => match self.decl_class(stack, name) {
                None => None,
                Some(rc) => {
                    self.cache.insert(name, Arc::clone(&rc));
                    Some(rc)
                }
            },
        }
    }
}
