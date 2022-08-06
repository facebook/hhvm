// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fmt::Debug;

use deps_rust::Dep;
use pos::ConstName;
use pos::FunName;
use pos::MethodName;
use pos::PropName;
use pos::TypeName;
use typing_deps_hash::DepType;

pub type Result<T, E = Error> = std::result::Result<T, E>;

#[derive(thiserror::Error, Debug)]
pub enum Error {}

// Implementations of `FoldedDeclProvider` need to be able to record
// dependencies (if needed). We do this by having the functions of this
// trait take a "who's asking?" symbol of this type.
#[derive(Clone, Copy, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum DeclName {
    Fun(FunName),
    Const(ConstName),
    Type(TypeName),
}

impl DeclName {
    pub fn hash1(&self) -> deps_rust::Dep {
        deps_rust::Dep::new(match self {
            DeclName::Fun(n) => typing_deps_hash::hash1(DepType::Fun, n.as_str().as_bytes()),
            DeclName::Const(n) => typing_deps_hash::hash1(DepType::GConst, n.as_str().as_bytes()),
            DeclName::Type(n) => typing_deps_hash::hash1(DepType::Type, n.as_str().as_bytes()),
        })
    }
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
#[derive(Clone, Copy, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
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
pub trait DepGraphWriter: Debug + Send + Sync {
    /// Record a dependency.
    // e.g. If class B extends A {} then A <- B (B depends on A). So here,
    // dependent is B and dependency is A.
    fn add_dependency(&self, dependent: DeclName, dependency: DependencyName) -> Result<()>;
}

/// Query dependency records.
pub trait DepGraphReader: Debug + Send + Sync {
    /// Retrieve dependents of a name.
    fn get_dependents(&self, dependency: DependencyName) -> Box<dyn Iterator<Item = Dep> + '_>;
}

/// A no-op implementation of the `DepGraphReader` & `DepGraphWriter` traits.
/// All registered edges are thrown away.
#[derive(Debug, Clone)]
pub struct NoDepGraph(());

impl NoDepGraph {
    pub fn new() -> Self {
        Self(())
    }
}

impl DepGraphWriter for NoDepGraph {
    fn add_dependency(&self, _dependent: DeclName, _dependency: DependencyName) -> Result<()> {
        Ok(())
    }
}

impl DepGraphReader for NoDepGraph {
    fn get_dependents(&self, _dependency: DependencyName) -> Box<dyn Iterator<Item = Dep>> {
        Box::new(std::iter::empty())
    }
}

/// Read & write dependency records.
pub trait DepGraph: DepGraphReader + DepGraphWriter {}
impl<T: DepGraphReader + DepGraphWriter> DepGraph for T {}
