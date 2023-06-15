(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open ServerTypeHierarchyTypes
open Typing_defs

let classish_kind_to_entryKind (kind : Ast_defs.classish_kind) : entryKind =
  let open Ast_defs in
  match kind with
  | Cclass _ -> Class
  | Cenum_class _ -> Enum
  | Cinterface -> Interface
  | Ctrait -> Trait
  | Cenum -> Enum

let decl_to_hierarchy ctx class_ : hierarchyEntry =
  let ancestors = Decl_provider.Class.all_ancestor_names class_ in
  let name = Decl_provider.Class.name class_ in
  let kind = classish_kind_to_entryKind (Decl_provider.Class.kind class_) in
  let pos =
    Naming_provider.resolve_position ctx (Decl_provider.Class.pos class_)
  in
  { name; kind; pos; ancestors }

let go_quarantined
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(line : int)
    ~(column : int) : ServerTypeHierarchyTypes.result =
  let { Tast_provider.Compute_tast.tast; _ } =
    Tast_provider.compute_tast_quarantined ~ctx ~entry
  in
  let symbols =
    IdentifySymbolService.go_quarantined ~ctx ~entry ~line ~column
  in
  let identity =
    List.find symbols ~f:(fun v ->
        match v.SymbolOccurrence.type_ with
        | SymbolOccurrence.Class _ -> true
        | _ -> false)
  in

  let env_and_ty =
    ServerInferType.human_friendly_type_at_pos
      ~under_dynamic:false
      ctx
      tast
      line
      column
  in
  match (identity, env_and_ty) with
  | (None, None) -> None
  | (Some sym, _) ->
    (* found a named entity, a class name *)
    let class_ = Decl_provider.get_class ctx sym.SymbolOccurrence.name in
    Option.map class_ ~f:(fun class_ -> decl_to_hierarchy ctx class_)
  | (_, Some (_env, ty)) ->
    (* type of an expression, look to see if we have a class to show here *)
    (match get_node ty with
    | Tclass ((_, c_name), _, _) ->
      let class_ = Decl_provider.get_class ctx c_name in
      Option.map class_ ~f:(fun class_ -> decl_to_hierarchy ctx class_)
    | _ -> None)

let json_of_hierarchy_entry (entry : hierarchyEntry) =
  Hh_json.JSON_Object
    [
      ("name", Hh_json.string_ entry.name);
      ("kind", Hh_json.string_ (show_entryKind entry.kind));
      ("pos", Hh_json.string_ (Pos.string_no_file entry.pos));
      ("ancestors", Hh_json.array_ Hh_json.string_ entry.ancestors);
    ]

let json_of_results ~(results : result) =
  match results with
  | None -> Hh_json.JSON_Object []
  | Some entry -> json_of_hierarchy_entry entry
