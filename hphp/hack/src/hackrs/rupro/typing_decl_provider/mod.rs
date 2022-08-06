// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fmt::Debug;
use std::rc::Rc;
use std::sync::Arc;

use defs::ClassType;
use depgraph_api::DeclName;
use pos::ConstName;
use pos::FunName;
use pos::MethodName;
use pos::PropName;
use pos::TypeName;
use ty::decl::ty::ConsistentKind;
use ty::decl::ConstDecl;
use ty::decl::EnumType;
use ty::decl::FunDecl;
use ty::decl::Requirement;
use ty::decl::Tparam;
use ty::decl::Ty;
use ty::decl::TypedefDecl;
use ty::decl::WhereConstraint;
use ty::decl_error::DeclError;
use ty::reason::Reason;

mod defs;
mod provider;

pub use provider::FoldingTypingDeclProvider;
pub use ty::local::ClassElt;

pub type Result<T, E = Error> = std::result::Result<T, E>;

#[derive(thiserror::Error, Debug)]
pub enum Error {
    #[error("{0}")]
    Folded(#[from] folded_decl_provider::Error),
}

#[derive(Debug)]
pub enum TypeDecl<R: Reason> {
    Class(Rc<dyn Class<R>>),
    Typedef(Arc<TypedefDecl<R>>),
}

/// The interface through which the typechecker accesses the type signatures of
/// all toplevel definitions. The class types exposed here are complete class
/// types--everything the typechecker might want to know about a class
/// declaration (whether the class is abstract, the types of its ancestors, the
/// types of all inherited methods and properties, etc.) is available through
/// the `Class` interface.
pub trait TypingDeclProvider<R: Reason>: Debug {
    /// Fetch the declaration of the toplevel function with the given name.
    fn get_fun(&self, dependent: DeclName, name: FunName) -> Result<Option<Arc<FunDecl<R>>>>;

    /// Fetch the declaration of the global constant with the given name.
    fn get_const(&self, dependent: DeclName, name: ConstName) -> Result<Option<Arc<ConstDecl<R>>>>;

    /// Fetch the declaration of the class or typedef with the given name.
    fn get_type(&self, dependent: DeclName, name: TypeName) -> Result<Option<TypeDecl<R>>>;

    /// Fetch the declaration of the typedef with the given name. If the given
    /// name is bound to a class rather than a typedef, return `None`.
    fn get_typedef(
        &self,
        dependent: DeclName,
        name: TypeName,
    ) -> Result<Option<Arc<TypedefDecl<R>>>> {
        Ok(self.get_type(dependent, name)?.and_then(|decl| match decl {
            TypeDecl::Typedef(td) => Some(td),
            TypeDecl::Class(..) => None,
        }))
    }

    /// Fetch the declaration of the class with the given name. If the given
    /// name is bound to a typedef rather than a class, return `None`.
    fn get_class(&self, dependent: DeclName, name: TypeName) -> Result<Option<Rc<dyn Class<R>>>> {
        Ok(self.get_type(dependent, name)?.and_then(|decl| match decl {
            TypeDecl::Class(cls) => Some(cls),
            TypeDecl::Typedef(..) => None,
        }))
    }
}

/// Represents the complete folded declaration of a class.
pub trait Class<R: Reason>: Debug {
    fn get_prop(&self, dependent: DeclName, name: PropName) -> Result<Option<Rc<ClassElt<R>>>>;

    fn get_static_prop(
        &self,
        dependent: DeclName,
        name: PropName,
    ) -> Result<Option<Rc<ClassElt<R>>>>;

    fn get_method(&self, dependent: DeclName, name: MethodName) -> Result<Option<Rc<ClassElt<R>>>>;

    fn get_static_method(
        &self,
        dependent: DeclName,
        name: MethodName,
    ) -> Result<Option<Rc<ClassElt<R>>>>;

    fn get_constructor(
        &self,
        dependent: DeclName,
    ) -> Result<(Option<Rc<ClassElt<R>>>, ConsistentKind)>;

    fn is_final(&self) -> bool;

    fn get_enum_type(&self) -> Option<&EnumType<R>>;
    fn get_tparams(&self) -> &[Tparam<R, Ty<R>>];
    fn decl_errors(&self) -> &[DeclError<R::Pos>];

    fn where_constraints(&self) -> &[WhereConstraint<Ty<R>>];
    fn upper_bounds_on_this_from_constraints(&self) -> Vec<Ty<R>>;
    fn all_ancestor_reqs(&self) -> &[Requirement<R>];
    fn upper_bounds_on_this(&self) -> Vec<Ty<R>>;
    fn get_ancestor(&self, super_name: &TypeName) -> Option<&Ty<R>>;
}
