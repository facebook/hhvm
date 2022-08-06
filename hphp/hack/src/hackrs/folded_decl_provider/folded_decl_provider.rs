// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fmt::Debug;
use std::sync::Arc;

use depgraph_api::DeclName;
use itertools::Itertools;
use pos::ConstName;
use pos::FunName;
use pos::MethodName;
use pos::PropName;
use pos::TypeName;
use ty::decl::ConstDecl;
use ty::decl::FoldedClass;
use ty::decl::FunDecl;
use ty::decl::Ty;
use ty::decl::TypedefDecl;
use ty::reason::Reason;

mod fold;
mod inherit;
mod provider;
mod subst;

pub use provider::LazyFoldedDeclProvider;
pub use subst::Substitution;

pub type Result<T, E = Error> = std::result::Result<T, E>;

#[derive(thiserror::Error, Debug)]
pub enum Error {
    #[error("{0}")]
    Shallow(#[from] shallow_decl_provider::Error),

    #[error("{0}")]
    Dependency(#[from] depgraph_api::Error),

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
///
/// Method and property types are omitted to reduce copying and overfetching in
/// `FoldedDeclProvider` implementations which store data in a serialized
/// format. We want to avoid copying the type of a method which is inherited by
/// 1000 classes into 1000 separate serialized blobs. To avoid this, we
/// serialize method types in a separate data store keyed by `(TypeName,
/// MethodName)`. Each method is stored only once, for the class which defined
/// the method. When looking up the type of an inherited method, a caller must
/// use the `origin` field of the method's `FoldedElement` to discover the class
/// in which the method was defined, invoke
/// `FoldedDeclProvider::get_shallow_method_type` to get the type of the method
/// as declared in that origin class, then use the `SubstContext` from the
/// descendant `FoldedClass` to instantiate that method type for the type
/// parameterization of the descendant class.
///
/// Implementations of member type accessors are expected to be consistent with
/// any `FoldedClass` values returned. For example, if a provider ever returns a
/// `FoldedClass` with a method "x" having origin "\\C", then that provider is
/// expected to return `Ok(Some(..))` or `Err` on _any_ subsequent invocation of
/// `get_shallow_method_type("\\C", "x")`. If a member cannot be found because
/// it was evicted from cache, and cannot be re-parsed from disk because the
/// source file was changed or deleted, folded decl providers are expected to
/// return `Err` rather than `None`. Simple implementations of
/// `FoldedDeclProvider` which return `None` in this scenario are not forbidden,
/// but they may violate invariants of users of `FoldedDeclProvider` (like
/// `TypingDeclProvider` implementations) in the event of disk changes.
pub trait FoldedDeclProvider<R: Reason>: Debug + Send + Sync {
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
    fn get_class(
        &self,
        dependent: DeclName,
        name: TypeName,
    ) -> Result<Option<Arc<FoldedClass<R>>>> {
        Ok(self.get_type(dependent, name)?.and_then(|decl| match decl {
            TypeDecl::Class(cls) => Some(cls),
            TypeDecl::Typedef(..) => None,
        }))
    }

    /// Fetch the type of the given property, as it was syntactically declared
    /// in the given class (i.e., returns `None` for inherited properties).
    ///
    /// Expected to be no slower than O(recursive_size_of_return_value) in the
    /// case of a cache hit. In other words, if the provider's cache backend
    /// stores data in a serialized format, implementations of this method
    /// should only deserialize the one property type, not the entire containing
    /// `ShallowClass`.
    fn get_shallow_property_type(
        &self,
        dependent: DeclName,
        class_name: TypeName,
        property_name: PropName,
    ) -> Result<Option<Ty<R>>>;

    /// Fetch the type of the given property, as it was syntactically declared
    /// in the given class (i.e., returns `None` for inherited properties).
    ///
    /// Expected to be no slower than O(recursive_size_of_return_value) in the
    /// case of a cache hit. In other words, if the provider's cache backend
    /// stores data in a serialized format, implementations of this method
    /// should only deserialize the one property type, not the entire containing
    /// `ShallowClass`.
    fn get_shallow_static_property_type(
        &self,
        dependent: DeclName,
        class_name: TypeName,
        property_name: PropName,
    ) -> Result<Option<Ty<R>>>;

    /// Fetch the type of the given method, as it was syntactically declared in
    /// the given class (i.e., returns `None` for inherited methods).
    ///
    /// Expected to be no slower than O(recursive_size_of_return_value) in the
    /// case of a cache hit. In other words, if the provider's cache backend
    /// stores data in a serialized format, implementations of this method
    /// should only deserialize the one method type, not the entire containing
    /// `ShallowClass`.
    fn get_shallow_method_type(
        &self,
        dependent: DeclName,
        class_name: TypeName,
        method_name: MethodName,
    ) -> Result<Option<Ty<R>>>;

    /// Fetch the type of the given method, as it was syntactically declared in
    /// the given class (i.e., returns `None` for inherited methods).
    ///
    /// Expected to be no slower than O(recursive_size_of_return_value) in the
    /// case of a cache hit. In other words, if the provider's cache backend
    /// stores data in a serialized format, implementations of this method
    /// should only deserialize the one method type, not the entire containing
    /// `ShallowClass`.
    fn get_shallow_static_method_type(
        &self,
        dependent: DeclName,
        class_name: TypeName,
        method_name: MethodName,
    ) -> Result<Option<Ty<R>>>;

    /// Fetch the type of the given constructor, as it was syntactically
    /// declared in the given class (i.e., returns `None` for inherited
    /// constructors).
    ///
    /// Expected to be no slower than O(recursive_size_of_return_value) in the
    /// case of a cache hit. In other words, if the provider's cache backend
    /// stores data in a serialized format, implementations of this method
    /// should only deserialize the one constructor type, not the entire
    /// containing `ShallowClass`.
    fn get_shallow_constructor_type(
        &self,
        dependent: DeclName,
        class_name: TypeName,
    ) -> Result<Option<Ty<R>>>;
}
