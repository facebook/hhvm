// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fmt::Debug;
use std::sync::Arc;

use itertools::Itertools;
use pos::TypeName;
use ty::decl::FoldedClass;
use ty::decl::TypedefDecl;
use ty::reason::Reason;

mod eager;
mod fold;
mod inherit;
mod provider;
mod subst;

pub use eager::EagerFoldedDeclProvider;
pub use fold::DeclFolder;
pub use provider::LazyFoldedDeclProvider;
pub use subst::Substitution;

pub type Result<T, E = Error> = std::result::Result<T, E>;

#[derive(thiserror::Error, Debug)]
pub enum Error {
    #[error("{0}")]
    Shallow(#[from] shallow_decl_provider::Error),

    #[error(
        "Failed to declare {class} because of error in ancestor {} (via {}): {error}",
        .parents.first().unwrap(),
        .parents.iter().rev().join(", "),
    )]
    Parent {
        class: TypeName,
        parents: Vec<TypeName>,
        #[source]
        error: Box<Error>,
    },

    #[error("Error in FoldedDeclProvider datastore: {0}")]
    Store(#[source] anyhow::Error),
}

#[derive(Clone, Debug)]
pub enum TypeDecl<R: Reason> {
    Class(Arc<FoldedClass<R>>),
    Typedef(Arc<TypedefDecl<R>>),
}

/// A get-or-compute interface for folded declarations. A folded class
/// declaration represents the near-complete type signature of that class; it
/// includes information about ancestors and metadata for all of the class'
/// members (including inherited members), but omits the types of all methods
/// and properties.
pub trait FoldedDeclProvider<R: Reason>: Debug + Send + Sync {
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
    fn get_class(&self, name: TypeName) -> Result<Option<Arc<FoldedClass<R>>>> {
        Ok(self.get_type(name)?.and_then(|decl| match decl {
            TypeDecl::Class(cls) => Some(cls),
            TypeDecl::Typedef(..) => None,
        }))
    }
}
