// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::decl_defs::{DeclTy, FoldedClass};
use crate::reason::Reason;
use pos::{MethodName, PropName, TypeName};
use std::fmt::Debug;
use std::sync::Arc;

mod fold;
mod inherit;
mod provider;
mod subst;

pub use provider::LazyFoldedDeclProvider;

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
pub trait FoldedDeclProvider<R: Reason>: Debug + Send + Sync {
    /// Fetch the folded declaration of the class with the given name. If the
    /// given name is bound to a typedef rather than a class, return `None`.
    fn get_class(&self, name: TypeName) -> Option<Arc<FoldedClass<R>>>;

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
        class_name: TypeName,
        property_name: PropName,
    ) -> Option<DeclTy<R>>;

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
        class_name: TypeName,
        property_name: PropName,
    ) -> Option<DeclTy<R>>;

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
        class_name: TypeName,
        method_name: MethodName,
    ) -> Option<DeclTy<R>>;

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
        class_name: TypeName,
        method_name: MethodName,
    ) -> Option<DeclTy<R>>;

    /// Fetch the type of the given constructor, as it was syntactically
    /// declared in the given class (i.e., returns `None` for inherited
    /// constructors).
    ///
    /// Expected to be no slower than O(recursive_size_of_return_value) in the
    /// case of a cache hit. In other words, if the provider's cache backend
    /// stores data in a serialized format, implementations of this method
    /// should only deserialize the one constructor type, not the entire
    /// containing `ShallowClass`.
    fn get_shallow_constructor_type(&self, class_name: TypeName) -> Option<DeclTy<R>>;
}
