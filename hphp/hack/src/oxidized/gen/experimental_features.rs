// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<650e75f630541f2a753536fa88e956d9>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::FromOcamlRepIn;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;
use strum::Display;
use strum::EnumIter;
use strum::EnumString;
use strum::IntoStaticStr;

#[allow(unused_imports)]
use crate::*;

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    EnumString,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (eq, ord, show)")]
#[repr(u8)]
pub enum FeatureStatus {
    Unstable,
    Preview,
    Migration,
    Deprecated,
    OngoingRelease,
}
impl TrivialDrop for FeatureStatus {}
arena_deserializer::impl_deserialize_in_arena!(FeatureStatus);

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Display,
    EnumIter,
    EnumString,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    FromOcamlRepIn,
    Hash,
    IntoStaticStr,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (eq, ord, show)")]
#[strum(serialize_all = "snake_case")]
#[repr(u8)]
pub enum FeatureName {
    UnionIntersectionTypeHints,
    ExpressionTrees,
    Readonly,
    ModuleReferences,
    ContextAliasDeclaration,
    ContextAliasDeclarationShort,
    TypeConstMultipleBounds,
    TypeConstSuperBound,
    ClassConstDefault,
    MethodTraitDiamond,
    UpcastExpression,
    RequireClass,
    RequireConstraint,
    NewtypeSuperBounds,
    Package,
    RequirePackage,
    CaseTypes,
    CaseTypeWhereClauses,
    ModuleLevelTraits,
    ModuleLevelTraitsExtensions,
    TypedLocalVariables,
    PipeAwait,
    MatchStatements,
    StrictSwitch,
    ClassType,
    FunctionReferences,
    FunctionTypeOptionalParams,
    SealedMethods,
    AwaitInSplice,
    OpenTuples,
    TypeSplat,
    ExpressionTreeNestedBindings,
    LikeTypeHints,
    ShapeDestructure,
    ExpressionTreeShapeCreation,
    NoDisjointUnion,
    SimpliHack,
    PolymorphicFunctionHints,
}
impl TrivialDrop for FeatureName {}
arena_deserializer::impl_deserialize_in_arena!(FeatureName);
