// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::Arc;

use datastore::Store;
use depgraph_api::DepGraphWriter;
use depgraph_api::DependencyName;
use oxidized::global_options::GlobalOptions;
use oxidized::naming_types::KindOfType;
use pos::ConstName;
use pos::FunName;
use pos::MethodName;
use pos::Positioned;
use pos::PropName;
use pos::TypeName;
use pos::TypeNameIndexMap;
use pos::TypeNameIndexSet;
use shallow_decl_provider::ShallowDeclProvider;
use ty::decl::ConstDecl;
use ty::decl::FoldedClass;
use ty::decl::FunDecl;
use ty::decl::ShallowClass;
use ty::decl::Ty;
use ty::decl_error::DeclError;
use ty::reason::Reason;

use super::fold::DeclFolder;
use super::DeclName;
use super::Error;
use super::Result;
use super::TypeDecl;

// note(sf, 2022-02-03): c.f. hphp/hack/src/decl/decl_folded_class.ml

/// A `FoldedDeclProvider` which, if the requested class name is not present in
/// its store, recursively computes the folded decl for that class by requesting
/// the shallow decls of that class and its ancestors from its
/// `ShallowDeclProvider`.
#[derive(Debug)]
pub struct LazyFoldedDeclProvider<R: Reason> {
    opts: Arc<GlobalOptions>,
    store: Arc<dyn Store<TypeName, Arc<FoldedClass<R>>>>,
    shallow_decl_provider: Arc<dyn ShallowDeclProvider<R>>,
    dependency_registrar: Arc<dyn DepGraphWriter>,
}

impl<R: Reason> LazyFoldedDeclProvider<R> {
    pub fn new(
        opts: Arc<GlobalOptions>,
        store: Arc<dyn Store<TypeName, Arc<FoldedClass<R>>>>,
        shallow_decl_provider: Arc<dyn ShallowDeclProvider<R>>,
        dependency_registrar: Arc<dyn DepGraphWriter>,
    ) -> Self {
        Self {
            opts,
            store,
            shallow_decl_provider,
            dependency_registrar,
        }
    }
}

impl<R: Reason> super::FoldedDeclProvider<R> for LazyFoldedDeclProvider<R> {
    fn get_fun(&self, _dependent: DeclName, name: FunName) -> Result<Option<Arc<FunDecl<R>>>> {
        Ok(self.shallow_decl_provider.get_fun(name)?)
    }

    fn get_const(
        &self,
        _dependent: DeclName,
        name: ConstName,
    ) -> Result<Option<Arc<ConstDecl<R>>>> {
        Ok(self.shallow_decl_provider.get_const(name)?)
    }

    fn get_type(&self, _dependent: DeclName, name: TypeName) -> Result<Option<TypeDecl<R>>> {
        match self.shallow_decl_provider.get_type_kind(name)? {
            None => Ok(None),
            Some(KindOfType::TTypedef) => Ok(Some(TypeDecl::Typedef(
                self.shallow_decl_provider.get_typedef(name)?.expect(
                    "got None after get_type_kind indicated a typedef with this name exists",
                ),
            ))),
            Some(KindOfType::TClass) => {
                let mut stack = Default::default();
                Ok(self
                    .get_folded_class_impl(&mut stack, name)?
                    .map(TypeDecl::Class))
            }
        }
    }

    fn get_shallow_property_type(
        &self,
        _dependent: DeclName,
        class_name: TypeName,
        property_name: PropName,
    ) -> Result<Option<Ty<R>>> {
        Ok(self
            .shallow_decl_provider
            .get_property_type(class_name, property_name)?)
    }

    fn get_shallow_static_property_type(
        &self,
        _dependent: DeclName,
        class_name: TypeName,
        property_name: PropName,
    ) -> Result<Option<Ty<R>>> {
        Ok(self
            .shallow_decl_provider
            .get_static_property_type(class_name, property_name)?)
    }

    fn get_shallow_method_type(
        &self,
        _dependent: DeclName,
        class_name: TypeName,
        method_name: MethodName,
    ) -> Result<Option<Ty<R>>> {
        Ok(self
            .shallow_decl_provider
            .get_method_type(class_name, method_name)?)
    }

    fn get_shallow_static_method_type(
        &self,
        _dependent: DeclName,
        class_name: TypeName,
        method_name: MethodName,
    ) -> Result<Option<Ty<R>>> {
        Ok(self
            .shallow_decl_provider
            .get_static_method_type(class_name, method_name)?)
    }

    fn get_shallow_constructor_type(
        &self,
        _dependent: DeclName,
        class_name: TypeName,
    ) -> Result<Option<Ty<R>>> {
        Ok(self
            .shallow_decl_provider
            .get_constructor_type(class_name)?)
    }
}

impl<R: Reason> LazyFoldedDeclProvider<R> {
    fn detect_cycle(
        &self,
        stack: &mut TypeNameIndexSet,
        errors: &mut Vec<DeclError<R::Pos>>,
        pos_id: &Positioned<TypeName, R::Pos>,
    ) -> bool {
        if stack.contains(&pos_id.id()) {
            errors.push(DeclError::CyclicClassDef(
                pos_id.pos().clone(),
                stack.iter().copied().collect(),
            ));
            true
        } else {
            false
        }
    }

    fn decl_class_type(
        &self,
        stack: &mut TypeNameIndexSet,
        errors: &mut Vec<DeclError<R::Pos>>,
        ty: &Ty<R>,
    ) -> Result<Option<(TypeName, Arc<FoldedClass<R>>)>> {
        let (_, pos_id, _) = ty.unwrap_class_type();
        if !self.detect_cycle(stack, errors, &pos_id) {
            if let Some(folded_decl) = self.get_folded_class_impl(stack, pos_id.id())? {
                return Ok(Some((pos_id.id(), folded_decl)));
            }
        }
        Ok(None)
    }

    fn parent_error(sc: &ShallowClass<R>, parent: &Ty<R>, err: Error) -> Error {
        // We tried to produce a decl of the parent of the given class but
        // failed. We capture this chain of events as a `Parent` error. The has
        // the effect of explaining that "we couldn't decl 'class' because we
        // couldn't decl 'parent' because ... x" ( where 'x' is the underlying
        // error like, the parent's php file is missing).
        let (_, parent_name, _) = parent.unwrap_class_type();
        match err {
            Error::Parent {
                class: _,
                mut parents,
                error,
            } => {
                parents.push(parent_name.id());
                Error::Parent {
                    class: sc.name.id(),
                    parents,
                    error,
                }
            }
            _ => Error::Parent {
                class: sc.name.id(),
                parents: vec![parent_name.id()],
                error: Box::new(err),
            },
        }
    }

    fn register_extends_dependency(&self, ty: &Ty<R>, sc: &ShallowClass<R>) -> Result<()> {
        let (_, p, _) = ty.unwrap_class_type();
        let child = sc.name.id();
        let parent = p.id();
        self.dependency_registrar
            .add_dependency(DeclName::Type(child), DependencyName::Extends(parent))?;
        Ok(())
    }

    // note(sf, 2022-03-02): c.f. Decl_folded_class.class_parents_decl
    fn decl_class_parents(
        &self,
        stack: &mut TypeNameIndexSet,
        errors: &mut Vec<DeclError<R::Pos>>,
        sc: &ShallowClass<R>,
    ) -> Result<TypeNameIndexMap<Arc<FoldedClass<R>>>> {
        (sc.extends.iter())
            .chain(sc.implements.iter())
            .chain(sc.uses.iter())
            .chain(sc.xhp_attr_uses.iter())
            .chain(sc.req_extends.iter())
            .chain(sc.req_implements.iter())
            .chain(
                (sc.enum_type.as_ref())
                    .map_or([].as_slice(), |et| &et.includes)
                    .iter(),
            )
            .chain(DeclFolder::stringish_object_parent(sc).iter())
            .map(|ty| {
                self.register_extends_dependency(ty, sc)?;
                self.decl_class_type(stack, errors, ty)
                    .map_err(|err| Self::parent_error(sc, ty, err))
            })
            .filter_map(Result::transpose)
            .collect()
    }

    fn decl_class(
        &self,
        stack: &mut TypeNameIndexSet,
        name: TypeName,
    ) -> Result<Option<Arc<FoldedClass<R>>>> {
        let mut errors = vec![];
        let shallow_class = match self.shallow_decl_provider.get_class(name)? {
            None => return Ok(None),
            Some(c) => c,
        };
        stack.insert(name);
        let parents = self.decl_class_parents(stack, &mut errors, &shallow_class)?;
        stack.remove(&name);
        Ok(Some(DeclFolder::decl_class(
            &self.opts,
            &*self.dependency_registrar,
            &shallow_class,
            &parents,
            errors,
        )?))
    }

    fn get_folded_class_impl(
        &self,
        stack: &mut TypeNameIndexSet,
        name: TypeName,
    ) -> Result<Option<Arc<FoldedClass<R>>>> {
        match self.store.get(name).map_err(Error::Store)? {
            Some(rc) => Ok(Some(rc)),
            None => match self.decl_class(stack, name)? {
                None => Ok(None),
                Some(rc) => {
                    self.store
                        .insert(name, Arc::clone(&rc))
                        .map_err(Error::Store)?;
                    Ok(Some(rc))
                }
            },
        }
    }
}
