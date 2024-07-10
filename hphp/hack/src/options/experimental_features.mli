(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* The logic to determine whether a feature is enabled is implemented in
   check_can_use_feature and the logic to determine whether the file-level
   attribute is allowed is in enable_experimental_feature, both in
   rust_parser_errors.rs *)

type feature_status =
  | Unstable
      (** An Unstable feature can only be enabled if the allow_unstable_features
        parser option is set, or in an .hhi. *)
  | Preview  (** A Preview feature can always be enabled. *)
  | Migration
  | Deprecated
  | OngoingRelease
      (** An OngoingRelease feature is always considered enabled in the runtime,
      but not the type checker where it still needs the attribute. *)
[@@deriving eq, ord, show]

(** Experimental features are enabled with a file attribute
    <file:__EnableUnstableFeatures('feature_name')>> where the feature name is
    one of these construcors written in snake_case. *)
type feature_name =
  | UnionIntersectionTypeHints
  | ClassLevelWhere
  | ExpressionTrees
  | Readonly
  | ModuleReferences
  | ClassConstDefault
  | TypeConstMultipleBounds
  | TypeConstSuperBound
  | TypeRefinements
  | ContextAliasDeclaration
  | ContextAliasDeclarationShort
  | MethodTraitDiamond
  | UpcastExpression
  | RequireClass
  | NewtypeSuperBounds
  | ExpressionTreeBlocks
  | Package
  | CaseTypes
  | ModuleLevelTraits
  | ModuleLevelTraitsExtensions
  | TypedLocalVariables
  | PipeAwait
  | MatchStatements
  | StrictSwitch
  | ClassType
  | FunctionReferences
  | FunctionTypeOptionalParams
  | ExpressionTreeMap
  | ExpressionTreeNest
  | SealedMethods
  | AwaitInSplice
[@@deriving eq, ord, show]

val feature_status_from_string : string -> feature_status option

val feature_name_from_string : string -> feature_name option
