// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::Arc;

use anyhow::Result;
use datastore::Store;
use pos::ConstName;
use pos::FunName;
use pos::MethodName;
use pos::ModuleName;
use pos::PropName;
use pos::TypeName;
use ty::decl::shallow::Decl;
use ty::decl::shallow::ModuleDecl;
use ty::decl::ConstDecl;
use ty::decl::FunDecl;
use ty::decl::ShallowClass;
use ty::decl::Ty;
use ty::decl::TypedefDecl;
use ty::reason::Reason;

/// A datastore for shallow declarations (i.e., the information we get from
/// decl-parsing a file). The backing datastores are permitted to evict their
/// contents at any time.
///
/// Consumers of a `ShallowDeclStore` expect the member-type accessors
/// (`get_method_type`, `get_constructor_type`, etc.) to be performant. For
/// instance, if our `Store` implementations store data in a serialized format,
/// looking up a method type should only deserialize that individual method, not
/// the entire `ShallowClass` containing that method declaration.
#[derive(Debug)]
pub struct ShallowDeclStore<R: Reason> {
    classes: Arc<dyn Store<TypeName, Arc<ShallowClass<R>>>>,
    typedefs: Arc<dyn Store<TypeName, Arc<TypedefDecl<R>>>>,
    funs: Arc<dyn Store<FunName, Arc<FunDecl<R>>>>,
    consts: Arc<dyn Store<ConstName, Arc<ConstDecl<R>>>>,
    modules: Arc<dyn Store<ModuleName, Arc<ModuleDecl<R>>>>,

    // The below tables are intended to be index tables for information stored
    // in the `classes` table (the underlying data is shared via the `Hc` in
    // `Ty`). When inserting or removing from the `classes` table, these
    // indices must be updated.
    properties: Arc<dyn Store<(TypeName, PropName), Ty<R>>>,
    static_properties: Arc<dyn Store<(TypeName, PropName), Ty<R>>>,
    methods: Arc<dyn Store<(TypeName, MethodName), Ty<R>>>,
    static_methods: Arc<dyn Store<(TypeName, MethodName), Ty<R>>>,
    constructors: Arc<dyn Store<TypeName, Ty<R>>>,
}

impl<R: Reason> ShallowDeclStore<R> {
    pub fn new(
        classes: Arc<dyn Store<TypeName, Arc<ShallowClass<R>>>>,
        typedefs: Arc<dyn Store<TypeName, Arc<TypedefDecl<R>>>>,
        funs: Arc<dyn Store<FunName, Arc<FunDecl<R>>>>,
        consts: Arc<dyn Store<ConstName, Arc<ConstDecl<R>>>>,
        modules: Arc<dyn Store<ModuleName, Arc<ModuleDecl<R>>>>,
        properties: Arc<dyn Store<(TypeName, PropName), Ty<R>>>,
        static_properties: Arc<dyn Store<(TypeName, PropName), Ty<R>>>,
        methods: Arc<dyn Store<(TypeName, MethodName), Ty<R>>>,
        static_methods: Arc<dyn Store<(TypeName, MethodName), Ty<R>>>,
        constructors: Arc<dyn Store<TypeName, Ty<R>>>,
    ) -> Self {
        Self {
            classes,
            typedefs,
            funs,
            consts,
            modules,
            properties,
            static_properties,
            methods,
            static_methods,
            constructors,
        }
    }

    /// Construct a `ShallowDeclStore` which looks up class members from the
    /// given `classes` table rather than maintaining separate member stores.
    /// Intended to be used with `Store` implementations which hold on to
    /// hash-consed `Ty`s in memory (rather than storing them in a
    /// serialized format), so that looking up individual members doesn't
    /// involve deserializing an entire `ShallowClass`.
    pub fn with_no_member_stores(
        classes: Arc<dyn Store<TypeName, Arc<ShallowClass<R>>>>,
        typedefs: Arc<dyn Store<TypeName, Arc<TypedefDecl<R>>>>,
        funs: Arc<dyn Store<FunName, Arc<FunDecl<R>>>>,
        consts: Arc<dyn Store<ConstName, Arc<ConstDecl<R>>>>,
        modules: Arc<dyn Store<ModuleName, Arc<ModuleDecl<R>>>>,
    ) -> Self {
        Self {
            properties: Arc::new(PropFinder {
                classes: Arc::clone(&classes),
            }),
            static_properties: Arc::new(StaticPropFinder {
                classes: Arc::clone(&classes),
            }),
            methods: Arc::new(MethodFinder {
                classes: Arc::clone(&classes),
            }),
            static_methods: Arc::new(StaticMethodFinder {
                classes: Arc::clone(&classes),
            }),
            constructors: Arc::new(ConstructorFinder {
                classes: Arc::clone(&classes),
            }),

            classes,
            typedefs,
            funs,
            consts,
            modules,
        }
    }

    pub fn add_decls(&self, decls: impl IntoIterator<Item = Decl<R>>) -> Result<()> {
        for decl in decls.into_iter() {
            match decl {
                Decl::Class(name, decl) => self.add_class(name, Arc::new(decl))?,
                Decl::Fun(name, decl) => self.funs.insert(name, Arc::new(decl))?,
                Decl::Typedef(name, decl) => self.typedefs.insert(name, Arc::new(decl))?,
                Decl::Const(name, decl) => self.consts.insert(name, Arc::new(decl))?,
                Decl::Module(name, decl) => self.modules.insert(name, Arc::new(decl))?,
            }
        }
        Ok(())
    }

    pub fn get_fun(&self, name: FunName) -> Result<Option<Arc<FunDecl<R>>>> {
        self.funs.get(name)
    }

    pub fn get_const(&self, name: ConstName) -> Result<Option<Arc<ConstDecl<R>>>> {
        self.consts.get(name)
    }

    pub fn get_module(&self, name: ModuleName) -> Result<Option<Arc<ModuleDecl<R>>>> {
        self.modules.get(name)
    }

    pub fn get_class(&self, name: TypeName) -> Result<Option<Arc<ShallowClass<R>>>> {
        self.classes.get(name)
    }

    pub fn get_typedef(&self, name: TypeName) -> Result<Option<Arc<TypedefDecl<R>>>> {
        self.typedefs.get(name)
    }

    pub fn get_property_type(
        &self,
        class_name: TypeName,
        property_name: PropName,
    ) -> Result<Option<Ty<R>>> {
        self.properties.get((class_name, property_name))
    }

    pub fn get_static_property_type(
        &self,
        class_name: TypeName,
        property_name: PropName,
    ) -> Result<Option<Ty<R>>> {
        self.static_properties.get((class_name, property_name))
    }

    pub fn get_method_type(
        &self,
        class_name: TypeName,
        method_name: MethodName,
    ) -> Result<Option<Ty<R>>> {
        self.methods.get((class_name, method_name))
    }

    pub fn get_static_method_type(
        &self,
        class_name: TypeName,
        method_name: MethodName,
    ) -> Result<Option<Ty<R>>> {
        self.static_methods.get((class_name, method_name))
    }

    pub fn get_constructor_type(&self, class_name: TypeName) -> Result<Option<Ty<R>>> {
        self.constructors.get(class_name)
    }

    fn add_class(&self, name: TypeName, cls: Arc<ShallowClass<R>>) -> Result<()> {
        let cid = cls.name.id();
        for prop in cls.props.iter().rev() {
            if let Some(ty) = &prop.ty {
                self.properties.insert((cid, prop.name.id()), ty.clone())?
            }
        }
        for prop in cls.static_props.iter().rev() {
            if let Some(ty) = &prop.ty {
                self.static_properties
                    .insert((cid, prop.name.id()), ty.clone())?
            }
        }
        for meth in cls.methods.iter().rev() {
            self.methods
                .insert((cid, meth.name.id()), meth.ty.clone())?
        }
        for meth in cls.static_methods.iter().rev() {
            self.static_methods
                .insert((cid, meth.name.id()), meth.ty.clone())?
        }
        if let Some(constructor) = &cls.constructor {
            self.constructors.insert(cid, constructor.ty.clone())?
        }
        self.classes.insert(name, cls)?;
        Ok(())
    }
}

/// Looks up props from the `classes` Store instead of storing them separately.
#[derive(Debug)]
struct PropFinder<R: Reason> {
    classes: Arc<dyn Store<TypeName, Arc<ShallowClass<R>>>>,
}

impl<R: Reason> Store<(TypeName, PropName), Ty<R>> for PropFinder<R> {
    fn get(&self, (class_name, property_name): (TypeName, PropName)) -> Result<Option<Ty<R>>> {
        Ok(self.classes.get(class_name)?.and_then(|cls| {
            cls.props.iter().rev().find_map(|prop| {
                if prop.name.id() == property_name {
                    prop.ty.clone()
                } else {
                    None
                }
            })
        }))
    }
    fn insert(&self, _: (TypeName, PropName), _: Ty<R>) -> Result<()> {
        Ok(())
    }
    fn remove_batch(&self, _: &mut dyn Iterator<Item = (TypeName, PropName)>) -> Result<()> {
        Ok(())
    }
}

/// Looks up props from the `classes` Store instead of storing them separately.
#[derive(Debug)]
struct StaticPropFinder<R: Reason> {
    classes: Arc<dyn Store<TypeName, Arc<ShallowClass<R>>>>,
}

impl<R: Reason> Store<(TypeName, PropName), Ty<R>> for StaticPropFinder<R> {
    fn get(&self, (class_name, property_name): (TypeName, PropName)) -> Result<Option<Ty<R>>> {
        Ok(self.classes.get(class_name)?.and_then(|cls| {
            cls.static_props.iter().rev().find_map(|prop| {
                if prop.name.id() == property_name {
                    prop.ty.clone()
                } else {
                    None
                }
            })
        }))
    }
    fn insert(&self, _: (TypeName, PropName), _: Ty<R>) -> Result<()> {
        Ok(())
    }
    fn remove_batch(&self, _: &mut dyn Iterator<Item = (TypeName, PropName)>) -> Result<()> {
        Ok(())
    }
}

/// Looks up methods from the `classes` Store instead of storing them separately.
#[derive(Debug)]
struct MethodFinder<R: Reason> {
    classes: Arc<dyn Store<TypeName, Arc<ShallowClass<R>>>>,
}

impl<R: Reason> Store<(TypeName, MethodName), Ty<R>> for MethodFinder<R> {
    fn get(&self, (class_name, method_name): (TypeName, MethodName)) -> Result<Option<Ty<R>>> {
        Ok(self.classes.get(class_name)?.and_then(|cls| {
            cls.methods.iter().rev().find_map(|meth| {
                if meth.name.id() == method_name {
                    Some(meth.ty.clone())
                } else {
                    None
                }
            })
        }))
    }
    fn insert(&self, _: (TypeName, MethodName), _: Ty<R>) -> Result<()> {
        Ok(())
    }
    fn remove_batch(&self, _: &mut dyn Iterator<Item = (TypeName, MethodName)>) -> Result<()> {
        Ok(())
    }
}

/// Looks up methods from the `classes` Store instead of storing them separately.
#[derive(Debug)]
struct StaticMethodFinder<R: Reason> {
    classes: Arc<dyn Store<TypeName, Arc<ShallowClass<R>>>>,
}

impl<R: Reason> Store<(TypeName, MethodName), Ty<R>> for StaticMethodFinder<R> {
    fn get(&self, (class_name, method_name): (TypeName, MethodName)) -> Result<Option<Ty<R>>> {
        Ok(self.classes.get(class_name)?.and_then(|cls| {
            cls.static_methods.iter().rev().find_map(|meth| {
                if meth.name.id() == method_name {
                    Some(meth.ty.clone())
                } else {
                    None
                }
            })
        }))
    }
    fn insert(&self, _: (TypeName, MethodName), _: Ty<R>) -> Result<()> {
        Ok(())
    }
    fn remove_batch(&self, _: &mut dyn Iterator<Item = (TypeName, MethodName)>) -> Result<()> {
        Ok(())
    }
}

/// Looks up constructors from the `classes` Store instead of storing them separately.
#[derive(Debug)]
struct ConstructorFinder<R: Reason> {
    classes: Arc<dyn Store<TypeName, Arc<ShallowClass<R>>>>,
}

impl<R: Reason> Store<TypeName, Ty<R>> for ConstructorFinder<R> {
    fn get(&self, class_name: TypeName) -> Result<Option<Ty<R>>> {
        Ok(self
            .classes
            .get(class_name)?
            .and_then(|cls| cls.constructor.as_ref().map(|meth| meth.ty.clone())))
    }
    fn insert(&self, _: TypeName, _: Ty<R>) -> Result<()> {
        Ok(())
    }
    fn remove_batch(&self, _: &mut dyn Iterator<Item = TypeName>) -> Result<()> {
        Ok(())
    }
}
