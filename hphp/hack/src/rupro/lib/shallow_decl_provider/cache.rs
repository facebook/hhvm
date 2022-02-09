// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::cache::Cache;
use crate::decl_defs::shallow::{self, ConstDecl, Decl, FunDecl, ShallowClass, TypedefDecl};
use crate::decl_defs::DeclTy;
use crate::reason::Reason;
use pos::{ConstName, FunName, MethodName, PropName, TypeName};
use std::sync::Arc;

#[derive(Clone, Debug)]
pub enum TypeDecl<R: Reason> {
    Class(Arc<ShallowClass<R>>),
    Typedef(Arc<TypedefDecl<R>>),
}

/// A cache for shallow declarations (i.e., the information we get from
/// decl-parsing a file). The backing caches are permitted to evict their
/// contents at any time.
///
/// Consumers of a `ShallowDeclCache` expect the member-type accessors
/// (`get_method_type`, `get_constructor_type`, etc.) to be performant. For
/// instance, if our `Cache` implementations store data in a serialized format,
/// looking up a method type should only deserialize that individual method, not
/// the entire `ShallowClass` containing that method declaration. The current
/// implementation of `ShallowDeclCache` does not satisfy this requirement, and
/// so it is only suitable for use with `Cache` backends which store hash-consed
/// decls in memory!
#[derive(Debug)]
pub struct ShallowDeclCache<R: Reason> {
    types: Box<dyn Cache<TypeName, TypeDecl<R>>>,
    funs: Box<dyn Cache<FunName, Arc<FunDecl<R>>>>,
    consts: Box<dyn Cache<ConstName, Arc<ConstDecl<R>>>>,
}

impl<R: Reason> ShallowDeclCache<R> {
    pub fn new(
        types: Box<dyn Cache<TypeName, TypeDecl<R>>>,
        funs: Box<dyn Cache<FunName, Arc<FunDecl<R>>>>,
        consts: Box<dyn Cache<ConstName, Arc<ConstDecl<R>>>>,
    ) -> Self {
        Self {
            types,
            funs,
            consts,
        }
    }

    pub fn with_no_eviction() -> Self {
        use crate::cache::NonEvictingCache;
        Self::new(
            Box::new(NonEvictingCache::default()),
            Box::new(NonEvictingCache::default()),
            Box::new(NonEvictingCache::default()),
        )
    }

    pub fn add_decls(&self, decls: Vec<shallow::Decl<R>>) {
        for decl in decls {
            match decl {
                Decl::Class(name, decl) => self.types.insert(name, TypeDecl::Class(Arc::new(decl))),
                Decl::Fun(name, decl) => self.funs.insert(name, Arc::new(decl)),
                Decl::Typedef(name, decl) => {
                    self.types.insert(name, TypeDecl::Typedef(Arc::new(decl)))
                }
                Decl::Const(name, decl) => self.consts.insert(name, Arc::new(decl)),
            }
        }
    }

    pub fn get_type(&self, name: TypeName) -> Option<TypeDecl<R>> {
        self.types.get(name)
    }

    pub fn get_fun(&self, name: FunName) -> Option<Arc<FunDecl<R>>> {
        self.funs.get(name)
    }

    pub fn get_const(&self, name: ConstName) -> Option<Arc<ConstDecl<R>>> {
        self.consts.get(name)
    }

    pub fn get_class(&self, name: TypeName) -> Option<Arc<ShallowClass<R>>> {
        self.types.get(name).and_then(|decl| match decl {
            TypeDecl::Class(cls) => Some(cls),
            TypeDecl::Typedef(..) => None,
        })
    }

    pub fn get_typedef(&self, name: TypeName) -> Option<Arc<TypedefDecl<R>>> {
        self.types.get(name).and_then(|decl| match decl {
            TypeDecl::Typedef(td) => Some(td),
            TypeDecl::Class(..) => None,
        })
    }

    pub fn get_property_type(
        &self,
        class_name: TypeName,
        property_name: PropName,
    ) -> Option<DeclTy<R>> {
        self.get_class(class_name).and_then(|cls| {
            cls.props.iter().rev().find_map(|prop| {
                if prop.name.id() == property_name {
                    prop.ty.clone()
                } else {
                    None
                }
            })
        })
    }

    pub fn get_static_property_type(
        &self,
        class_name: TypeName,
        property_name: PropName,
    ) -> Option<DeclTy<R>> {
        self.get_class(class_name).and_then(|cls| {
            cls.static_props.iter().rev().find_map(|prop| {
                if prop.name.id() == property_name {
                    prop.ty.clone()
                } else {
                    None
                }
            })
        })
    }

    pub fn get_method_type(
        &self,
        class_name: TypeName,
        method_name: MethodName,
    ) -> Option<DeclTy<R>> {
        self.get_class(class_name).and_then(|cls| {
            cls.methods.iter().rev().find_map(|meth| {
                if meth.name.id() == method_name {
                    Some(meth.ty.clone())
                } else {
                    None
                }
            })
        })
    }

    pub fn get_static_method_type(
        &self,
        class_name: TypeName,
        method_name: MethodName,
    ) -> Option<DeclTy<R>> {
        self.get_class(class_name).and_then(|cls| {
            cls.static_methods.iter().rev().find_map(|meth| {
                if meth.name.id() == method_name {
                    Some(meth.ty.clone())
                } else {
                    None
                }
            })
        })
    }

    pub fn get_constructor_type(&self, class_name: TypeName) -> Option<DeclTy<R>> {
        self.get_class(class_name)
            .and_then(|cls| cls.constructor.as_ref().map(|meth| meth.ty.clone()))
    }
}
