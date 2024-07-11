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
  | ContextAliasDeclaration
  | ContextAliasDeclarationShort
  | TypeConstMultipleBounds
  | TypeConstSuperBound
  | ClassConstDefault
  | TypeRefinements
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
      ("context_alias_declaration", ContextAliasDeclaration);
      ("context_alias_declaration_short", ContextAliasDeclarationShort);
      ("type_const_multiple_bounds", TypeConstMultipleBounds);
      ("type_const_super_bound", TypeConstSuperBound);
      ("class_const_default", ClassConstDefault);
      ("type_refinements", TypeRefinements);
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

(** Return the hard-coded status of the feature. This information will be moved to configuration. *)
let get_feature_status_deprecated name =
  match name with
  | UnionIntersectionTypeHints -> Unstable
  | ClassLevelWhere -> Unstable
  | ExpressionTrees -> Unstable
  | Readonly -> Preview
  | ModuleReferences -> Unstable
  | ContextAliasDeclaration -> Unstable
  | ContextAliasDeclarationShort -> Preview
  | TypeConstMultipleBounds -> Preview
  | TypeConstSuperBound -> Unstable
  | ClassConstDefault -> Migration
  | TypeRefinements -> OngoingRelease
  | MethodTraitDiamond -> OngoingRelease
  | UpcastExpression -> Unstable
  | RequireClass -> OngoingRelease
  | NewtypeSuperBounds -> Unstable
  | ExpressionTreeBlocks -> OngoingRelease
  | Package -> OngoingRelease
  | CaseTypes -> Preview
  | ModuleLevelTraits -> OngoingRelease
  | ModuleLevelTraitsExtensions -> OngoingRelease
  | TypedLocalVariables -> Preview
  | PipeAwait -> Preview
  | MatchStatements -> Unstable
  | StrictSwitch -> Unstable
  | ClassType -> Unstable
  | FunctionReferences -> Unstable
  | FunctionTypeOptionalParams -> OngoingRelease
  | ExpressionTreeMap -> OngoingRelease
  | ExpressionTreeNest -> Preview
  | SealedMethods -> Unstable
  | AwaitInSplice -> Preview

let parse_experimental_feature (name_string, status_json) =
  let status_string = Hh_json.get_string_exn status_json in
  match
    ( feature_name_from_string name_string,
      feature_status_from_string status_string )
  with
  | (Some name, Some status) ->
    let hard_coded_status = get_feature_status_deprecated name in
    (* For now, force the config to be consistent with the hard coded status. *)
    if equal_feature_status status hard_coded_status then
      (name, status)
    else
      failwith
        (Format.sprintf
           "Experimental feature status mismatch for feature %s: %s in config must be %s during experimental feature config roll-out"
           name_string
           status_string
           (show_feature_status hard_coded_status))
  | (None, _) -> failwith ("Invalid experimental feature name: " ^ name_string)
  | (_, None) ->
    failwith ("Invalid experimental feature status: " ^ status_string)
