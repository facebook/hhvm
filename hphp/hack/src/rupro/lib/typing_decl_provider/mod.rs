// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::decl_defs::{ConstDecl, FunDecl, TypedefDecl};
use crate::reason::Reason;
use defs::ClassType;
use pos::{ConstName, FunName, MethodName, PropName, TypeName};
use std::fmt::Debug;
use std::sync::Arc;

mod defs;
mod provider;

pub use crate::typing_defs::ClassElt;
pub use provider::FoldingTypingDeclProvider;

pub type Result<T, E = Error> = std::result::Result<T, E>;

#[derive(thiserror::Error, Debug)]
pub enum Error {
    #[error("{0}")]
    Folded(#[from] crate::folded_decl_provider::Error),
}

#[derive(Debug)]
pub enum TypeDecl<R: Reason> {
    Class(Arc<dyn Class<R>>),
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
    fn get_fun(&self, name: FunName) -> Result<Option<Arc<FunDecl<R>>>>;

    /// Fetch the declaration of the global constant with the given name.
    fn get_const(&self, name: ConstName) -> Result<Option<Arc<ConstDecl<R>>>>;

    /// Fetch the declaration of the class or typedef with the given name.
    fn get_type(&self, name: TypeName) -> Result<Option<TypeDecl<R>>>;

    /// Fetch the declaration of the typedef with the given name. If the given
    /// name is bound to a class rather than a typedef, return `None`.
    fn get_typedef(&self, name: TypeName) -> Result<Option<Arc<TypedefDecl<R>>>> {
        Ok(self.get_type(name)?.and_then(|decl| match decl {
            TypeDecl::Typedef(td) => Some(td),
            TypeDecl::Class(..) => None,
        }))
    }

    /// Fetch the declaration of the class with the given name. If the given
    /// name is bound to a typedef rather than a class, return `None`.
    fn get_class(&self, name: TypeName) -> Result<Option<Arc<dyn Class<R>>>> {
        Ok(self.get_type(name)?.and_then(|decl| match decl {
            TypeDecl::Class(cls) => Some(cls),
            TypeDecl::Typedef(..) => None,
        }))
    }
}

/// Represents the complete folded declaration of a class.
pub trait Class<R: Reason>: Debug {
    fn get_prop(&self, name: PropName) -> Result<Option<Arc<ClassElt<R>>>>;
    fn get_static_prop(&self, name: PropName) -> Result<Option<Arc<ClassElt<R>>>>;
    fn get_method(&self, name: MethodName) -> Result<Option<Arc<ClassElt<R>>>>;
    fn get_static_method(&self, name: MethodName) -> Result<Option<Arc<ClassElt<R>>>>;
    fn get_constructor(&self) -> Result<Option<Arc<ClassElt<R>>>>;
}
