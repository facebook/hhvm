// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fmt::Debug;
use std::sync::Arc;

use oxidized::naming_types::KindOfType;
use pos::ConstName;
use pos::FunName;
use pos::ModuleName;
use pos::RelativePath;
use pos::TypeName;
use ty::decl::ShallowClass;
use ty::decl::shallow::ConstDecl;
use ty::decl::shallow::FunDecl;
use ty::decl::shallow::ModuleDecl;
use ty::decl::shallow::TypedefDecl;
use ty::reason::Reason;

mod provider;
mod store;

pub use provider::EagerShallowDeclProvider;
pub use provider::LazyShallowDeclProvider;
pub use store::ShallowDeclStore;

pub type Result<T, E = Error> = std::result::Result<T, E>;

#[derive(thiserror::Error, Debug)]
pub enum Error {
    #[error("Failed to parse decls in {path:?}: {file_provider_error}")]
    DeclParse {
        path: RelativePath,
        #[source]
        file_provider_error: anyhow::Error,
    },
    #[error("Unexpected error: {0}")]
    Unexpected(#[from] anyhow::Error),
}

#[derive(Clone, Debug, serde::Serialize, serde::Deserialize)]
#[serde(bound = "R: Reason")]
pub enum TypeDecl<R: Reason> {
    Class(Arc<ShallowClass<R>>),
    Typedef(Arc<TypedefDecl<R>>),
}

/// A get-or-compute interface for shallow decls (i.e., the type signature
/// information syntactically available in a file; the output of the
/// decl-parser).
///
/// Consumers of a `ShallowDeclProvider` expect the member-type accessors
/// (`get_method_type`, `get_constructor_type`, etc.) to be performant. For
/// instance, if our `Store` implementations store data in a serialized format,
/// looking up a method type should only deserialize that individual method, not
/// the entire `ShallowClass` containing that method declaration.
///
/// Quick lookup of method and property types is useful because types of methods
/// and properties are omitted in the `FoldedClass` representation. This is done
/// to reduce copying and overfetching in `FoldedDeclProvider` implementations
/// which store data in a serialized format: we want to avoid copying the type
/// of a method which is inherited by 1000 classes into 1000 separate serialized
/// blobs. To avoid this, we serialize method types in a separate data store
/// keyed by `(TypeName, MethodName)`. Each method is stored only once, for the
/// class which defined the method. Inheritors must look up the method type
/// using the name of the "origin" class which defined it.
pub trait ShallowDeclProvider<R: Reason>: Debug + Send + Sync {
    /// Fetch the declaration of the toplevel function with the given name.
    fn get_fun(&self, name: FunName) -> Result<Option<Arc<FunDecl<R>>>>;

    /// Fetch the declaration of the global constant with the given name.
    fn get_const(&self, name: ConstName) -> Result<Option<Arc<ConstDecl<R>>>>;

    /// Fetch the declaration of the module with the given name.
    fn get_module(&self, name: ModuleName) -> Result<Option<Arc<ModuleDecl<R>>>>;

    /// Indicate whether the type with the given name is a typedef or class.
    fn get_type_kind(&self, name: TypeName) -> Result<Option<KindOfType>>;

    /// Fetch the declaration of the class or typedef with the given name.
    fn get_type(&self, name: TypeName) -> Result<Option<TypeDecl<R>>>;

    /// Fetch the declaration of the typedef with the given name. If the given
    /// name is bound to a class rather than a typedef, return `None`.
    fn get_typedef(&self, name: TypeName) -> Result<Option<Arc<TypedefDecl<R>>>>;

    /// Fetch the declaration of the class with the given name. If the given
    /// name is bound to a typedef rather than a class, return `None`.
    fn get_class(&self, name: TypeName) -> Result<Option<Arc<ShallowClass<R>>>>;
}
