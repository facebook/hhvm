// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::decl_defs::shallow::TypedefDecl;
use crate::reason::Reason;
use defs::ClassType;
use pos::{MethodName, PropName, TypeName};
use std::fmt::Debug;
use std::sync::Arc;

mod defs;
mod provider;

pub use crate::typing_defs::ClassElt;
pub use provider::FoldingTypingDeclProvider;

/// The interface through which the typechecker accesses the type signatures of
/// all toplevel definitions. The class types exposed here are complete class
/// types--everything the typechecker might want to know about a class
/// declaration (whether the class is abstract, the types of its ancestors, the
/// types of all inherited methods and properties, etc.) is available through
/// the `Class` interface.
pub trait TypingDeclProvider<R: Reason>: Debug {
    fn get_class(&self, name: TypeName) -> Option<Arc<dyn Class<R>>>;
    fn get_class_or_typedef(&self, name: TypeName) -> Option<TypeDecl<R>>;
}

#[derive(Debug)]
pub enum TypeDecl<R: Reason> {
    Class(Arc<dyn Class<R>>),
    Typedef(Arc<TypedefDecl<R>>),
}

/// Represents the complete folded declaration of a class.
pub trait Class<R: Reason>: Debug {
    fn get_prop(&self, name: PropName) -> Option<Arc<ClassElt<R>>>;
    fn get_static_prop(&self, name: PropName) -> Option<Arc<ClassElt<R>>>;
    fn get_method(&self, name: MethodName) -> Option<Arc<ClassElt<R>>>;
    fn get_static_method(&self, name: MethodName) -> Option<Arc<ClassElt<R>>>;
    fn get_constructor(&self) -> Option<Arc<ClassElt<R>>>;
}
