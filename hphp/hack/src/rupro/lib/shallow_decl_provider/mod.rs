// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::decl_defs::{
    shallow::{ConstDecl, FunDecl, TypedefDecl},
    DeclTy, ShallowClass,
};
use crate::reason::Reason;
use pos::{ConstName, FunName, MethodName, PropName, TypeName};
use std::fmt::Debug;
use std::sync::Arc;

mod cache;
mod provider;

pub use cache::ShallowDeclCache;
pub use provider::{EagerShallowDeclProvider, LazyShallowDeclProvider};

#[derive(Clone, Debug)]
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
/// instance, if our `Cache` implementations store data in a serialized format,
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
    fn get_fun(&self, name: FunName) -> Option<Arc<FunDecl<R>>>;

    /// Fetch the declaration of the global constant with the given name.
    fn get_const(&self, name: ConstName) -> Option<Arc<ConstDecl<R>>>;

    /// Fetch the declaration of the class or typedef with the given name.
    fn get_type(&self, name: TypeName) -> Option<TypeDecl<R>>;

    /// Fetch the declaration of the typedef with the given name. If the given
    /// name is bound to a class rather than a typedef, return `None`.
    fn get_typedef(&self, name: TypeName) -> Option<Arc<TypedefDecl<R>>> {
        self.get_type(name).and_then(|decl| match decl {
            TypeDecl::Typedef(td) => Some(td),
            TypeDecl::Class(..) => None,
        })
    }

    /// Fetch the declaration of the class with the given name. If the given
    /// name is bound to a typedef rather than a class, return `None`.
    fn get_class(&self, name: TypeName) -> Option<Arc<ShallowClass<R>>> {
        self.get_type(name).and_then(|decl| match decl {
            TypeDecl::Class(cls) => Some(cls),
            TypeDecl::Typedef(..) => None,
        })
    }

    /// Fetch the type of the property with the given name from the given
    /// shallow class. When multiple properties are declared with the same name,
    /// return the one that appears last in the source text.
    ///
    /// Expected to be no slower than O(recursive_size_of_return_value) in the
    /// case of a cache hit. In other words, if the provider's cache backend
    /// stores data in a serialized format, implementations of this method
    /// should only deserialize the one property type, not the entire containing
    /// `ShallowClass`.
    fn get_property_type(&self, class_name: TypeName, property_name: PropName)
        -> Option<DeclTy<R>>;

    /// Fetch the type of the property with the given name from the given
    /// shallow class. When multiple properties are declared with the same name,
    /// return the one that appears last in the source text.
    ///
    /// Expected to be no slower than O(recursive_size_of_return_value) in the
    /// case of a cache hit. In other words, if the provider's cache backend
    /// stores data in a serialized format, implementations of this method
    /// should only deserialize the one property type, not the entire containing
    /// `ShallowClass`.
    fn get_static_property_type(
        &self,
        class_name: TypeName,
        property_name: PropName,
    ) -> Option<DeclTy<R>>;

    /// Fetch the type of the method with the given name from the given shallow
    /// class. When multiple methods are declared with the same name, return the
    /// one that appears last in the source text.
    ///
    /// Expected to be no slower than O(recursive_size_of_return_value) in the
    /// case of a cache hit. In other words, if the provider's cache backend
    /// stores data in a serialized format, implementations of this method
    /// should only deserialize the one method type, not the entire containing
    /// `ShallowClass`.
    fn get_method_type(&self, class_name: TypeName, method_name: MethodName) -> Option<DeclTy<R>>;

    /// Fetch the type of the method with the given name from the given shallow
    /// class. When multiple methods are declared with the same name, return the
    /// one that appears last in the source text.
    ///
    /// Expected to be no slower than O(recursive_size_of_return_value) in the
    /// case of a cache hit. In other words, if the provider's cache backend
    /// stores data in a serialized format, implementations of this method
    /// should only deserialize the one method type, not the entire containing
    /// `ShallowClass`.
    fn get_static_method_type(
        &self,
        class_name: TypeName,
        method_name: MethodName,
    ) -> Option<DeclTy<R>>;

    /// Fetch the type of the constructor declared in the given shallow class.
    ///
    /// Expected to be no slower than O(recursive_size_of_return_value) in the
    /// case of a cache hit. In other words, if the provider's cache backend
    /// stores data in a serialized format, implementations of this method
    /// should only deserialize the one constructor type, not the entire
    /// containing `ShallowClass`.
    fn get_constructor_type(&self, class_name: TypeName) -> Option<DeclTy<R>>;
}
