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

let get_snippet ctx name ty =
  let env = Typing_env_types.empty ctx Relative_path.default ~droot:None in
  let (_, ty) = Typing_phase.localize_no_subst env ~ignore_errors:true ty in
  let tast_env = Tast_env.empty ctx in
  match get_node ty with
  | Tfun ft ->
    let params = ft.ft_params in
    let n = List.length params in
    let is_variadic = get_ft_variadic ft in
    let param_snippet =
      params
      |> List.mapi ~f:(fun i param ->
             let name =
               match param.fp_name with
               | Some pn -> pn
               | None -> ""
             in
             let prefix =
               if is_variadic && i + 1 = n then
                 "..."
               else
                 ""
             in
             let ty = Tast_env.print_ty tast_env param.fp_type in
             Printf.sprintf "%s %s%s" ty prefix name)
      |> String.concat ~sep:", "
    in
    let ret_type = Tast_env.print_ty tast_env ft.ft_ret in
    Printf.sprintf "%s(%s): %s" name param_snippet ret_type
  | _ -> Printf.sprintf "%s: %s" name (Tast_env.print_ty tast_env ty)

let get_members ctx class_ : memberEntry list =
  let class_etl_to_member_entry kind (name, member) : memberEntry =
    let snippet =
      get_snippet ctx name (Lazy.force member.Typing_defs.ce_type)
    in
    let pos =
      Lazy.force member.Typing_defs.ce_pos
      |> Naming_provider.resolve_position ctx
      |> Pos.to_absolute
    in
    let origin = Utils.strip_ns member.Typing_defs.ce_origin in
    { name; kind; snippet; pos; origin }
  in
  let class_const_to_member_entry
      ((name, const) : string * Typing_defs.class_const) : memberEntry =
    let snippet = get_snippet ctx name const.Typing_defs.cc_type in
    let pos =
      const.Typing_defs.cc_pos
      |> Naming_provider.resolve_position ctx
      |> Pos.to_absolute
    in
    let origin = Utils.strip_ns const.Typing_defs.cc_origin in
    { name; kind = ServerTypeHierarchyTypes.Const; snippet; pos; origin }
  in
  (Decl_provider.Class.methods class_
  |> List.map ~f:(class_etl_to_member_entry ServerTypeHierarchyTypes.Method))
  @ (Decl_provider.Class.smethods class_
    |> List.map ~f:(class_etl_to_member_entry ServerTypeHierarchyTypes.SMethod)
    )
  @ (Decl_provider.Class.props class_
    |> List.map ~f:(class_etl_to_member_entry ServerTypeHierarchyTypes.Property)
    )
  @ (Decl_provider.Class.sprops class_
    |> List.map
         ~f:(class_etl_to_member_entry ServerTypeHierarchyTypes.SProperty))
  @ (Decl_provider.Class.consts class_
    |> List.map ~f:class_const_to_member_entry)

let classish_kind_to_entryKind (kind : Ast_defs.classish_kind) : entryKind =
  let open Ast_defs in
  match kind with
  | Cclass _ -> Class
  | Cenum_class _ -> Enum
  | Cinterface -> Interface
  | Ctrait -> Trait
  | Cenum -> Enum

let get_ancestor_entry ctx name : ancestorEntry =
  let class_ = Decl_provider.get_class ctx name in
  match class_ with
  | Decl_entry.DoesNotExist
  | Decl_entry.NotYetAvailable ->
    AncestorName (Utils.strip_ns name)
  | Decl_entry.Found class_ ->
    AncestorDetails
      {
        name = Utils.strip_ns name;
        kind = classish_kind_to_entryKind (Decl_provider.Class.kind class_);
        pos =
          Decl_provider.Class.pos class_
          |> Naming_provider.resolve_position ctx
          |> Pos.to_absolute;
      }

let decl_to_hierarchy ctx class_ : hierarchyEntry =
  let name = Utils.strip_ns (Decl_provider.Class.name class_) in
  let kind = classish_kind_to_entryKind (Decl_provider.Class.kind class_) in
  let pos =
    Decl_provider.Class.pos class_
    |> Naming_provider.resolve_position ctx
    |> Pos.to_absolute
  in
  let members = get_members ctx class_ in
  let ancestors =
    Decl_provider.Class.all_ancestor_names class_
    |> List.map ~f:(get_ancestor_entry ctx)
    |> List.filter ~f:(function
           | AncestorDetails e ->
             (not (phys_equal e.kind Interface)) || phys_equal kind Interface
           | _ -> false)
  in
  { name; kind; pos; ancestors; members }

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
    let class_ =
      Decl_provider.get_class ctx sym.SymbolOccurrence.name
      |> Decl_entry.to_option
    in
    Option.map class_ ~f:(fun class_ -> decl_to_hierarchy ctx class_)
  | (_, Some (_env, ty)) ->
    (* type of an expression, look to see if we have a class to show here *)
    (match get_node ty with
    | Tclass ((_, c_name), _, _) ->
      let class_ = Decl_provider.get_class ctx c_name |> Decl_entry.to_option in
      Option.map class_ ~f:(fun class_ -> decl_to_hierarchy ctx class_)
    | _ -> None)

let json_of_member_entry (entry : memberEntry) =
  Hh_json.JSON_Object
    [
      ("name", Hh_json.string_ entry.name);
      ("snippet", Hh_json.string_ entry.snippet);
      ( "kind",
        Hh_json.string_ (ServerTypeHierarchyTypes.show_memberKind entry.kind) );
      ("pos", Hh_json.string_ (Pos.string_no_file entry.pos));
      ("origin", Hh_json.string_ entry.origin);
    ]

let json_of_ancestor_entry (entry : ancestorEntry) =
  match entry with
  | AncestorName name -> Hh_json.string_ name
  | AncestorDetails entry ->
    Hh_json.JSON_Object
      [
        ("name", Hh_json.string_ entry.name);
        ("kind", Hh_json.string_ (show_entryKind entry.kind));
        ("pos", Hh_json.string_ (Pos.string_no_file entry.pos));
      ]

let json_of_hierarchy_entry (entry : hierarchyEntry) =
  Hh_json.JSON_Object
    [
      ("name", Hh_json.string_ entry.name);
      ("kind", Hh_json.string_ (show_entryKind entry.kind));
      ("pos", Hh_json.string_ (Pos.string_no_file entry.pos));
      ("ancestors", Hh_json.array_ json_of_ancestor_entry entry.ancestors);
      ("members", Hh_json.array_ json_of_member_entry entry.members);
    ]

let json_of_results ~(results : result) =
  match results with
  | None -> Hh_json.JSON_Object []
  | Some entry -> json_of_hierarchy_entry entry
