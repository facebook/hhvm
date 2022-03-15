// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use pos::{ConstName, FunName, MethodName, PropName, TypeName};
use std::fmt::Debug;

pub type Result<T, E = Error> = std::result::Result<T, E>;

#[derive(thiserror::Error, Debug)]
pub enum Error {}

// Implementations of `FoldedDeclProvider` need to be able to record
// dependencies (if needed). We do this by having the functions of this
// trait take a "who's asking?" symbol of this type.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub enum DeclName {
    Fun(FunName),
    Const(ConstName),
    Type(TypeName),
}

impl From<FunName> for DeclName {
    fn from(name: FunName) -> Self {
        Self::Fun(name)
    }
}

impl From<ConstName> for DeclName {
    fn from(name: ConstName) -> Self {
        Self::Const(name)
    }
}

impl From<TypeName> for DeclName {
    fn from(name: TypeName) -> Self {
        Self::Type(name)
    }
}

// nb(sf, 2022-03-15): c.f. ` Typing_deps.Dep.variant`
#[derive(Clone, Debug, PartialEq, Eq, Hash)]
/// A node in the dependency graph that, when changed, must recheck all of its
/// dependents.
pub enum DependencyName {
    /// Represents another class depending on a class via an inheritance-like
    /// mechanism (`extends`, `implements`, `use`, `require extends`, `require
    /// implements`, etc.)
    Extends(TypeName),
    /// Represents something depending on a class constant.
    Const(ConstName),
    /// Represents something depending on a class constructor.
    Constructor(TypeName),
    /// Represents something depending on a class's instance property.
    Prop(TypeName, PropName),
    /// Represents something depending on a class's static property.
    StaticProp(TypeName, PropName),
    /// Represents something depending on a class's instance method.
    Method(TypeName, MethodName),
    /// Represents something depending on a class's static method.
    StaticMethod(TypeName, MethodName),
    /// Represents something depending on all members of a class. Particularly
    /// useful for switch exhaustiveness-checking. We establish a dependency on
    /// all members of an enum in that case.
    AllMembers(TypeName),
    /// Represents something depending on a global constant.
    GConst(ConstName),
    /// Represents something depending on a global function.
    Fun(FunName),
    /// Represents something depending on a class/typedef/trait/interface
    Type(TypeName),
}

/// Organize and administer dependency records.
pub trait DependencyRegistrar: Debug + Send + Sync {
    /// Record a dependency.
    fn add_dependency(&mut self, dependency: DependencyName, dependent: DeclName) -> Result<()>;
}
