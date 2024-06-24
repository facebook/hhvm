(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type feature_status =
  | Unstable
  | Preview
  | Migration
  | Deprecated
  | OngoingRelease
[@@deriving eq, ord, show]

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
