// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<c896f8c905d4fa29044ed35f6201cb44>>
//
// To regenerate this file, run:
//   buck run @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:oxidized_regen

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
    /// An Unstable feature can only be enabled if the allow_unstable_features
    /// parser option is set, or in an .hhi.
    Unstable,
    /// A Preview feature can always be enabled.
    Preview,
    Migration,
    Deprecated,
    /// An OngoingRelease feature is always considered enabled.
    OngoingRelease,
}
impl TrivialDrop for FeatureStatus {}
arena_deserializer::impl_deserialize_in_arena!(FeatureStatus);

/// Experimental features are enabled with a file attribute
/// <<file:__EnableUnstableFeatures('feature_name')>> where the feature name is
/// one of these construcors written in snake_case.
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
    ProtectedInternal,
    PolymorphicLambda,
    ExpressionTreeCoalesceOperator,
    ExpressionTreeNullsafeObjGet,
    /// "named_parameters": can use named parameters in function declarations
    /// `function f(int $x, named int $y);
    NamedParameters,
    /// "named_parameters_use":
    /// - will cover both named arguments and named parameters in types.
    /// - will not cover named parameters in function declarations. That will
    /// be covered by the "named_parameters" feature. This enables us to
    /// do staged rollout of named parameters in declarations without making
    /// calling such function awkward.
    /// ```
    /// f(x=4);
    /// function take((function(int, named int $b)) $f): void {}
    /// ```
    NamedParametersUse,
}
impl TrivialDrop for FeatureName {}
arena_deserializer::impl_deserialize_in_arena!(FeatureName);
