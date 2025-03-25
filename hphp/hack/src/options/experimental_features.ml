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
      ("method_trait_diamond", MethodTraitDiamond);
      ("upcast_expression", UpcastExpression);
      ("require_class", RequireClass);
      ("require_constraint", RequireConstraint);
      ("newtype_super_bounds", NewtypeSuperBounds);
      ("package", Package);
      ("require_package", RequirePackage);
      ("case_types", CaseTypes);
      ("case_type_where_clauses", CaseTypeWhereClauses);
      ("module_level_traits", ModuleLevelTraits);
      ("module_level_traits_extensions", ModuleLevelTraitsExtensions);
      ("typed_local_variables", TypedLocalVariables);
      ("pipe_await", PipeAwait);
      ("match_statements", MatchStatements);
      ("strict_switch", StrictSwitch);
      ("class_type", ClassType);
      ("function_references", FunctionReferences);
      ("function_type_optional_params", FunctionTypeOptionalParams);
      ("sealed_methods", SealedMethods);
      ("await_in_splice", AwaitInSplice);
      ("open_tuples", OpenTuples);
      ("type_splat", TypeSplat);
      ("expression_tree_nested_bindings", ExpressionTreeNestedBindings);
      ("like_type_hints", LikeTypeHints);
      ("shape_destructure", ShapeDestructure);
      ("expression_tree_shape_creation", ExpressionTreeShapeCreation);
      ("no_disjoint_union", NoDisjointUnion);
      ("simplihack", SimpliHack);
      ("polymorphic_function_hints", PolymorphicFunctionHints);
    ]

let feature_name_from_string s = SMap.find_opt s feature_name_map

(** Return the hard-coded status of the feature. This information will be moved to configuration. *)
external get_feature_status_deprecated : feature_name -> feature_status
  = "get_feature_status_deprecated"

let feature_more_restrictive ~more ~less =
  match (more, less) with
  | (Unstable, Preview)
  | (Unstable, OngoingRelease)
  | (Preview, OngoingRelease) ->
    true
  | _ -> false

let feature_more_restrictive_or_eq ~more ~less =
  equal_feature_status more less || feature_more_restrictive ~more ~less

let parse_experimental_feature (name_string, status_json) =
  let status_string = Hh_json.get_string_exn status_json in
  match
    ( feature_name_from_string name_string,
      feature_status_from_string status_string )
  with
  | (Some name, Some status) ->
    let hard_coded_status = get_feature_status_deprecated name in
    (* For now, force the config to be consistent with the hard coded status. *)
    if feature_more_restrictive_or_eq ~more:status ~less:hard_coded_status then
      (name_string, status)
    else
      failwith
        (Format.sprintf
           "Experimental feature status mismatch for feature %s: %s in config must be %s (or more restrictive) during experimental feature config roll-out"
           name_string
           status_string
           (show_feature_status hard_coded_status))
  | (None, _) -> failwith ("Invalid experimental feature name: " ^ name_string)
  | (_, None) ->
    failwith ("Invalid experimental feature status: " ^ status_string)
