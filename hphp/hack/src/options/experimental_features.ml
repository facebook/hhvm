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
  | ExpressionTrees
  | Readonly
  | ModuleReferences
  | ContextAliasDeclaration
  | ContextAliasDeclarationShort
  | TypeConstMultipleBounds
  | TypeConstSuperBound
  | ClassConstDefault
  | UpcastExpression
  | RequireConstraint
  | NewtypeSuperBounds
  | CaseTypes
  | CaseTypeWhereClauses
  | RecursiveCaseTypes
  | ModuleLevelTraits
  | ModuleLevelTraitsExtensions
  | PipeAwait
  | MatchStatements
  | StrictSwitch
  | ClassType
  | FunctionReferences
  | FunctionTypeOptionalParams
  | AwaitInSplice
  | OpenTuples
  | TypeSplat
  | ExpressionTreeNestedBindings
  | LikeTypeHints
  | ShapeAndTupleDestructuring
  | ExpressionTreeShapeCreation
  | NoDisjointUnion
  | SimpliHack
  | PolymorphicFunctionHints
  | PolymorphicLambda
  | ExpressionTreeCoalesceOperator
  | ExpressionTreeNullsafeObjGet
  | NamedParameters
  | XhpTypeConstants
  | CapturePipeVariables
  | AllowExtendedAwaitSyntax
  | AllowConditionalAwaitSyntax
  | ExpressionTreeHackArrays
  | ShapeFieldPunning
  | RepresentableAs
  | ClassAliasesEverywhere
  | TestFeature
  | WithRefinementAlias
[@@deriving eq, ord, show]

let feature_name_map =
  List.fold_left
    ~init:SMap.empty
    ~f:(fun m (k, v) -> SMap.add k v m)
    [
      ("union_intersection_type_hints", UnionIntersectionTypeHints);
      ("expression_trees", ExpressionTrees);
      ("readonly", Readonly);
      ("module_references", ModuleReferences);
      ("context_alias_declaration", ContextAliasDeclaration);
      ("context_alias_declaration_short", ContextAliasDeclarationShort);
      ("type_const_multiple_bounds", TypeConstMultipleBounds);
      ("type_const_super_bound", TypeConstSuperBound);
      ("class_const_default", ClassConstDefault);
      ("upcast_expression", UpcastExpression);
      ("require_constraint", RequireConstraint);
      ("newtype_super_bounds", NewtypeSuperBounds);
      ("case_types", CaseTypes);
      ("case_type_where_clauses", CaseTypeWhereClauses);
      ("recursive_case_types", RecursiveCaseTypes);
      ("module_level_traits", ModuleLevelTraits);
      ("module_level_traits_extensions", ModuleLevelTraitsExtensions);
      ("pipe_await", PipeAwait);
      ("match_statements", MatchStatements);
      ("strict_switch", StrictSwitch);
      ("class_type", ClassType);
      ("function_references", FunctionReferences);
      ("function_type_optional_params", FunctionTypeOptionalParams);
      ("await_in_splice", AwaitInSplice);
      ("open_tuples", OpenTuples);
      ("type_splat", TypeSplat);
      ("expression_tree_nested_bindings", ExpressionTreeNestedBindings);
      ("like_type_hints", LikeTypeHints);
      ("shape_and_tuple_destructuring", ShapeAndTupleDestructuring);
      ("expression_tree_shape_creation", ExpressionTreeShapeCreation);
      ("no_disjoint_union", NoDisjointUnion);
      ("simpli_hack", SimpliHack);
      ("polymorphic_function_hints", PolymorphicFunctionHints);
      ("polymorphic_lambda", PolymorphicLambda);
      ("expression_tree_coalesce_operator", ExpressionTreeCoalesceOperator);
      ("expression_tree_nullsafe_obj_get", ExpressionTreeNullsafeObjGet);
      ("named_parameters", NamedParameters);
      ("xhp_type_constants", XhpTypeConstants);
      ("capture_pipe_variables", CapturePipeVariables);
      ("allow_extended_await_syntax", AllowExtendedAwaitSyntax);
      ("allow_conditional_await_syntax", AllowConditionalAwaitSyntax);
      ("expression_tree_hack_arrays", ExpressionTreeHackArrays);
      ("shape_field_punning", ShapeFieldPunning);
      ("representable_as", RepresentableAs);
      ("class_aliases_everywhere", ClassAliasesEverywhere);
      ("test_feature", TestFeature);
      ("with_refinement_alias", WithRefinementAlias);
    ]

let feature_name_from_string s = SMap.find_opt s feature_name_map

let parse_experimental_feature (name_string, status_json) =
  let status_string =
    match status_json with
    | `String s -> s
    | _ -> failwith "expected string for experimental feature status"
  in
  match
    ( feature_name_from_string name_string,
      feature_status_from_string status_string )
  with
  | (Some _name, Some status) -> (name_string, status)
  | (None, _) -> failwith ("Invalid experimental feature name: " ^ name_string)
  | (_, None) ->
    failwith ("Invalid experimental feature status: " ^ status_string)
