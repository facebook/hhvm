(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type feature_status =
  | Unstable
  | Preview
  | Migration
  | Deprecated
  | OngoingRelease
[@@deriving eq, ord, show]

let feature_status_map =
  List.fold_left
    ~init:SMap.empty
    ~f:(fun m (k, v) -> SMap.add k v m)
    [
      ("Unstable", Unstable);
      ("Preview", Preview);
      ("Migration", Migration);
      ("Deprecated", Deprecated);
      ("OngoingRelease", OngoingRelease);
    ]

let feature_status_from_string s = SMap.find_opt s feature_status_map

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

let feature_name_map =
  List.fold_left
    ~init:SMap.empty
    ~f:(fun m (k, v) -> SMap.add k v m)
    [
      ("union_intersection_type_hints", UnionIntersectionTypeHints);
      ("class_level_where", ClassLevelWhere);
      ("expression_trees", ExpressionTrees);
      ("readonly", Readonly);
      ("module_references", ModuleReferences);
      ("class_const_default", ClassConstDefault);
      ("type_const_multiple_bounds", TypeConstMultipleBounds);
      ("type_const_super_bound", TypeConstSuperBound);
      ("type_refinements", TypeRefinements);
      ("context_alias_declaration", ContextAliasDeclaration);
      ("context_alias_declaration_short", ContextAliasDeclarationShort);
      ("method_trait_diamond", MethodTraitDiamond);
      ("upcast_expression", UpcastExpression);
      ("require_class", RequireClass);
      ("newtype_super_bounds", NewtypeSuperBounds);
      ("expression_tree_blocks", ExpressionTreeBlocks);
      ("package", Package);
      ("case_types", CaseTypes);
      ("module_level_traits", ModuleLevelTraits);
      ("module_level_traits_extensions", ModuleLevelTraitsExtensions);
      ("typed_local_variables", TypedLocalVariables);
      ("pipe_await", PipeAwait);
      ("match_statements", MatchStatements);
      ("strict_switch", StrictSwitch);
      ("class_type", ClassType);
      ("function_references", FunctionReferences);
      ("function_type_optional_params", FunctionTypeOptionalParams);
      ("expression_tree_map", ExpressionTreeMap);
      ("expression_tree_nest", ExpressionTreeNest);
      ("sealed_methods", SealedMethods);
      ("await_in_splice", AwaitInSplice);
    ]

let feature_name_from_string s = SMap.find_opt s feature_name_map
