// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::Arc;

use datastore::Store;
use depgraph_api::DepGraphWriter;
use depgraph_api::DependencyName;
use oxidized::naming_types::KindOfType;
use pos::ConstName;
use pos::FunName;
use pos::MethodName;
use pos::ModuleName;
use pos::PropName;
use pos::TypeName;
use shallow_decl_provider::ShallowDeclProvider;
use ty::decl::ConstDecl;
use ty::decl::FoldedClass;
use ty::decl::FunDecl;
use ty::decl::ModuleDecl;
use ty::decl::Ty;
use ty::reason::Reason;

use super::DeclName;
use super::Error;
use super::Result;
use super::TypeDecl;

/// A `FoldedDeclProvider` which assumes that all extant decls have eagerly been
/// inserted in its store. It returns `None` when asked for a decl which is not
/// present in the store.
#[derive(Debug)]
pub struct EagerFoldedDeclProvider<R: Reason> {
    store: Arc<dyn Store<TypeName, Arc<FoldedClass<R>>>>,
    shallow_decl_provider: Arc<dyn ShallowDeclProvider<R>>,
    dependency_registrar: Arc<dyn DepGraphWriter>,
}

impl<R: Reason> EagerFoldedDeclProvider<R> {
    pub fn new(
        store: Arc<dyn Store<TypeName, Arc<FoldedClass<R>>>>,
        shallow_decl_provider: Arc<dyn ShallowDeclProvider<R>>,
        dependency_registrar: Arc<dyn DepGraphWriter>,
    ) -> Self {
        Self {
            store,
            shallow_decl_provider,
            dependency_registrar,
        }
    }
}

impl<R: Reason> super::FoldedDeclProvider<R> for EagerFoldedDeclProvider<R> {
    fn get_fun(&self, dependent: DeclName, name: FunName) -> Result<Option<Arc<FunDecl<R>>>> {
        self.dependency_registrar
            .add_dependency(dependent, name.into())?;
        Ok(self.shallow_decl_provider.get_fun(name)?)
    }

    fn get_const(&self, dependent: DeclName, name: ConstName) -> Result<Option<Arc<ConstDecl<R>>>> {
        self.dependency_registrar
            .add_dependency(dependent, name.into())?;
        Ok(self.shallow_decl_provider.get_const(name)?)
    }

    fn get_module(
        &self,
        dependent: DeclName,
        name: ModuleName,
    ) -> Result<Option<Arc<ModuleDecl<R>>>> {
        self.dependency_registrar
            .add_dependency(dependent, name.into())?;
        Ok(self.shallow_decl_provider.get_module(name)?)
    }

    fn get_type(&self, dependent: DeclName, name: TypeName) -> Result<Option<TypeDecl<R>>> {
        self.dependency_registrar
            .add_dependency(dependent, name.into())?;
        match self.shallow_decl_provider.get_type_kind(name)? {
            None => Ok(None),
            Some(KindOfType::TTypedef) => Ok(Some(TypeDecl::Typedef(
                self.shallow_decl_provider.get_typedef(name)?.expect(
                    "got None after get_type_kind indicated a typedef with this name exists",
                ),
            ))),
            Some(KindOfType::TClass) => match self.store.get(name).map_err(Error::Store)? {
                Some(c) => Ok(Some(TypeDecl::Class(c))),
                None => Ok(None),
            },
        }
    }

    fn get_shallow_property_type(
        &self,
        dependent: DeclName,
        class_name: TypeName,
        property_name: PropName,
    ) -> Result<Option<Ty<R>>> {
        self.dependency_registrar
            .add_dependency(dependent, DependencyName::Prop(class_name, property_name))?;
        Ok(self
            .shallow_decl_provider
            .get_property_type(class_name, property_name)?)
    }

    fn get_shallow_static_property_type(
        &self,
        dependent: DeclName,
        class_name: TypeName,
        property_name: PropName,
    ) -> Result<Option<Ty<R>>> {
        self.dependency_registrar.add_dependency(
            dependent,
            DependencyName::StaticProp(class_name, property_name),
        )?;
        Ok(self
            .shallow_decl_provider
            .get_static_property_type(class_name, property_name)?)
    }

    fn get_shallow_method_type(
        &self,
        dependent: DeclName,
        class_name: TypeName,
        method_name: MethodName,
    ) -> Result<Option<Ty<R>>> {
        self.dependency_registrar
            .add_dependency(dependent, DependencyName::Method(class_name, method_name))?;
        Ok(self
            .shallow_decl_provider
            .get_method_type(class_name, method_name)?)
    }

    fn get_shallow_static_method_type(
        &self,
        dependent: DeclName,
        class_name: TypeName,
        method_name: MethodName,
    ) -> Result<Option<Ty<R>>> {
        self.dependency_registrar.add_dependency(
            dependent,
            DependencyName::StaticMethod(class_name, method_name),
        )?;
        Ok(self
            .shallow_decl_provider
            .get_static_method_type(class_name, method_name)?)
    }

    fn get_shallow_constructor_type(
        &self,
        dependent: DeclName,
        class_name: TypeName,
    ) -> Result<Option<Ty<R>>> {
        self.dependency_registrar
            .add_dependency(dependent, DependencyName::Constructor(class_name))?;
        Ok(self
            .shallow_decl_provider
            .get_constructor_type(class_name)?)
    }
}
