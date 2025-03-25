(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* The logic to determine whether a feature is enabled is implemented in
   `can_use` and the logic to determine whether the file-level attribute is
   allowed is in `enable`, both in experimental_features_impl.rs *)

type feature_status =
  | Unstable
      (** An Unstable feature can only be enabled if the allow_unstable_features
        parser option is set, or in an .hhi. *)
  | Preview  (** A Preview feature can always be enabled. *)
  | Migration
  | Deprecated
  | OngoingRelease
      (** An OngoingRelease feature is always considered enabled. *)
[@@deriving eq, ord, show]

(** Experimental features are enabled with a file attribute
    <<file:__EnableUnstableFeatures('feature_name')>> where the feature name is
    one of these construcors written in snake_case. *)
type feature_name =
  | UnionIntersectionTypeHints
  | ExpressionTrees
  | Readonly
  | ModuleReferences
  | ContextAliasDeclaration
  | ContextAliasDeclarationShort
  | TypeConstMultipleBounds
  | TypeConstSuperBound
  | ClassConstDefault
  | MethodTraitDiamond
  | UpcastExpression
  | RequireClass
  | RequireConstraint
  | NewtypeSuperBounds
  | Package
  | RequirePackage
  | CaseTypes
  | CaseTypeWhereClauses
  | ModuleLevelTraits
  | ModuleLevelTraitsExtensions
  | TypedLocalVariables
  | PipeAwait
  | MatchStatements
  | StrictSwitch
  | ClassType
  | FunctionReferences
  | FunctionTypeOptionalParams
  | SealedMethods
  | AwaitInSplice
  | OpenTuples
  | TypeSplat
  | ExpressionTreeNestedBindings
  | LikeTypeHints
  | ShapeDestructure
  | ExpressionTreeShapeCreation
  | NoDisjointUnion
  | SimpliHack
  | PolymorphicFunctionHints
[@@deriving eq, ord, show]

val feature_status_from_string : string -> feature_status option

val feature_name_from_string : string -> feature_name option

(** Convert string/json pair to feature name/status. Fail if the name/status aren't valid. *)
val parse_experimental_feature :
  string * Hh_json.json -> string * feature_status
