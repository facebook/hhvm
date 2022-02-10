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
/// the entire `ShallowClass` containing that method declaration.
#[derive(Debug)]
pub struct ShallowDeclCache<R: Reason> {
    types: Arc<dyn Cache<TypeName, TypeDecl<R>>>,
    funs: Box<dyn Cache<FunName, Arc<FunDecl<R>>>>,
    consts: Box<dyn Cache<ConstName, Arc<ConstDecl<R>>>>,

    // The below tables are intended to be index tables for information stored
    // in the `types` table (the underlying data is shared via the `Hc` in
    // `DeclTy`). When inserting or removing from the `types` table, these
    // indices must be updated.
    properties: Box<dyn Cache<(TypeName, PropName), DeclTy<R>>>,
    static_properties: Box<dyn Cache<(TypeName, PropName), DeclTy<R>>>,
    methods: Box<dyn Cache<(TypeName, MethodName), DeclTy<R>>>,
    static_methods: Box<dyn Cache<(TypeName, MethodName), DeclTy<R>>>,
    constructors: Box<dyn Cache<TypeName, DeclTy<R>>>,
}

impl<R: Reason> ShallowDeclCache<R> {
    pub fn new(
        types: Arc<dyn Cache<TypeName, TypeDecl<R>>>,
        funs: Box<dyn Cache<FunName, Arc<FunDecl<R>>>>,
        consts: Box<dyn Cache<ConstName, Arc<ConstDecl<R>>>>,
        properties: Box<dyn Cache<(TypeName, PropName), DeclTy<R>>>,
        static_properties: Box<dyn Cache<(TypeName, PropName), DeclTy<R>>>,
        methods: Box<dyn Cache<(TypeName, MethodName), DeclTy<R>>>,
        static_methods: Box<dyn Cache<(TypeName, MethodName), DeclTy<R>>>,
        constructors: Box<dyn Cache<TypeName, DeclTy<R>>>,
    ) -> Self {
        Self {
            types,
            funs,
            consts,
            properties,
            static_properties,
            methods,
            static_methods,
            constructors,
        }
    }

    pub fn with_no_eviction() -> Self {
        use crate::cache::NonEvictingCache;
        Self::with_no_member_caches(
            Arc::new(NonEvictingCache::default()),
            Box::new(NonEvictingCache::default()),
            Box::new(NonEvictingCache::default()),
        )
    }

    /// Construct a `ShallowDeclCache` which looks up class members from the
    /// given `types` table rather than maintaining separate member caches.
    /// Intended to be used with `Cache` implementations which hold on to
    /// hash-consed `DeclTy`s in memory (rather than storing them in a
    /// serialized format), so that looking up individual members doesn't
    /// involve deserializing an entire `ShallowClass`.
    pub fn with_no_member_caches(
        types: Arc<dyn Cache<TypeName, TypeDecl<R>>>,
        funs: Box<dyn Cache<FunName, Arc<FunDecl<R>>>>,
        consts: Box<dyn Cache<ConstName, Arc<ConstDecl<R>>>>,
    ) -> Self {
        Self {
            properties: Box::new(PropFinder {
                types: Arc::clone(&types),
            }),
            static_properties: Box::new(StaticPropFinder {
                types: Arc::clone(&types),
            }),
            methods: Box::new(MethodFinder {
                types: Arc::clone(&types),
            }),
            static_methods: Box::new(StaticMethodFinder {
                types: Arc::clone(&types),
            }),
            constructors: Box::new(ConstructorFinder {
                types: Arc::clone(&types),
            }),

            types,
            funs,
            consts,
        }
    }

    pub fn add_decls(&self, decls: Vec<shallow::Decl<R>>) {
        for decl in decls {
            match decl {
                Decl::Class(name, decl) => self.add_class(name, Arc::new(decl)),
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
        self.properties.get((class_name, property_name))
    }

    pub fn get_static_property_type(
        &self,
        class_name: TypeName,
        property_name: PropName,
    ) -> Option<DeclTy<R>> {
        self.static_properties.get((class_name, property_name))
    }

    pub fn get_method_type(
        &self,
        class_name: TypeName,
        method_name: MethodName,
    ) -> Option<DeclTy<R>> {
        self.methods.get((class_name, method_name))
    }

    pub fn get_static_method_type(
        &self,
        class_name: TypeName,
        method_name: MethodName,
    ) -> Option<DeclTy<R>> {
        self.static_methods.get((class_name, method_name))
    }

    pub fn get_constructor_type(&self, class_name: TypeName) -> Option<DeclTy<R>> {
        self.constructors.get(class_name)
    }

    fn add_class(&self, name: TypeName, cls: Arc<ShallowClass<R>>) {
        let cid = cls.name.id();
        for prop in &cls.props {
            if let Some(ty) = &prop.ty {
                self.properties.insert((cid, prop.name.id()), ty.clone())
            }
        }
        for prop in &cls.static_props {
            if let Some(ty) = &prop.ty {
                self.static_properties
                    .insert((cid, prop.name.id()), ty.clone())
            }
        }
        for meth in &cls.methods {
            self.methods.insert((cid, meth.name.id()), meth.ty.clone())
        }
        for meth in &cls.static_methods {
            self.static_methods
                .insert((cid, meth.name.id()), meth.ty.clone())
        }
        if let Some(constructor) = &cls.constructor {
            self.constructors.insert(cid, constructor.ty.clone())
        }
        self.types.insert(name, TypeDecl::Class(cls))
    }
}

/// Looks up props from the `types` cache instead of storing them separately.
#[derive(Debug)]
struct PropFinder<R: Reason> {
    types: Arc<dyn Cache<TypeName, TypeDecl<R>>>,
}

impl<R: Reason> Cache<(TypeName, PropName), DeclTy<R>> for PropFinder<R> {
    fn get(&self, (class_name, property_name): (TypeName, PropName)) -> Option<DeclTy<R>> {
        self.types
            .get(class_name)
            .and_then(|decl| match decl {
                TypeDecl::Class(cls) => Some(cls),
                TypeDecl::Typedef(..) => None,
            })
            .and_then(|cls| {
                cls.props.iter().rev().find_map(|prop| {
                    if prop.name.id() == property_name {
                        prop.ty.clone()
                    } else {
                        None
                    }
                })
            })
    }
    fn insert(&self, _: (TypeName, PropName), _: DeclTy<R>) {}
}

/// Looks up props from the `types` cache instead of storing them separately.
#[derive(Debug)]
struct StaticPropFinder<R: Reason> {
    types: Arc<dyn Cache<TypeName, TypeDecl<R>>>,
}

impl<R: Reason> Cache<(TypeName, PropName), DeclTy<R>> for StaticPropFinder<R> {
    fn get(&self, (class_name, property_name): (TypeName, PropName)) -> Option<DeclTy<R>> {
        self.types
            .get(class_name)
            .and_then(|decl| match decl {
                TypeDecl::Class(cls) => Some(cls),
                TypeDecl::Typedef(..) => None,
            })
            .and_then(|cls| {
                cls.static_props.iter().rev().find_map(|prop| {
                    if prop.name.id() == property_name {
                        prop.ty.clone()
                    } else {
                        None
                    }
                })
            })
    }
    fn insert(&self, _: (TypeName, PropName), _: DeclTy<R>) {}
}

/// Looks up methods from the `types` cache instead of storing them separately.
#[derive(Debug)]
struct MethodFinder<R: Reason> {
    types: Arc<dyn Cache<TypeName, TypeDecl<R>>>,
}

impl<R: Reason> Cache<(TypeName, MethodName), DeclTy<R>> for MethodFinder<R> {
    fn get(&self, (class_name, method_name): (TypeName, MethodName)) -> Option<DeclTy<R>> {
        self.types
            .get(class_name)
            .and_then(|decl| match decl {
                TypeDecl::Class(cls) => Some(cls),
                TypeDecl::Typedef(..) => None,
            })
            .and_then(|cls| {
                cls.methods.iter().rev().find_map(|meth| {
                    if meth.name.id() == method_name {
                        Some(meth.ty.clone())
                    } else {
                        None
                    }
                })
            })
    }
    fn insert(&self, _: (TypeName, MethodName), _: DeclTy<R>) {}
}

/// Looks up methods from the `types` cache instead of storing them separately.
#[derive(Debug)]
struct StaticMethodFinder<R: Reason> {
    types: Arc<dyn Cache<TypeName, TypeDecl<R>>>,
}

impl<R: Reason> Cache<(TypeName, MethodName), DeclTy<R>> for StaticMethodFinder<R> {
    fn get(&self, (class_name, method_name): (TypeName, MethodName)) -> Option<DeclTy<R>> {
        self.types
            .get(class_name)
            .and_then(|decl| match decl {
                TypeDecl::Class(cls) => Some(cls),
                TypeDecl::Typedef(..) => None,
            })
            .and_then(|cls| {
                cls.static_methods.iter().rev().find_map(|meth| {
                    if meth.name.id() == method_name {
                        Some(meth.ty.clone())
                    } else {
                        None
                    }
                })
            })
    }
    fn insert(&self, _: (TypeName, MethodName), _: DeclTy<R>) {}
}

/// Looks up constructors from the `types` cache instead of storing them separately.
#[derive(Debug)]
struct ConstructorFinder<R: Reason> {
    types: Arc<dyn Cache<TypeName, TypeDecl<R>>>,
}

impl<R: Reason> Cache<TypeName, DeclTy<R>> for ConstructorFinder<R> {
    fn get(&self, class_name: TypeName) -> Option<DeclTy<R>> {
        self.types
            .get(class_name)
            .and_then(|decl| match decl {
                TypeDecl::Class(cls) => Some(cls),
                TypeDecl::Typedef(..) => None,
            })
            .and_then(|cls| cls.constructor.as_ref().map(|meth| meth.ty.clone()))
    }
    fn insert(&self, _: TypeName, _: DeclTy<R>) {}
}
