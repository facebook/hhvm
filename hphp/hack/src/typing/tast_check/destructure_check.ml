(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

type shape_info = {
  shape_is_open: bool;
  shape_required_fields: Typing_defs.TShapeMap.key list;
  shape_all_fields: Typing_defs.TShapeMap.key list;
}

type tuple_info = {
  tuple_arity: int option;
  tuple_has_variadic_tail: bool;
}

let extract_shape_info
    (s_fields :
      Typing_defs.locl_phase Typing_defs.shape_field_type
      Typing_defs.TShapeMap.t)
    (s_unknown_value : Typing_defs.locl_ty) : shape_info =
  {
    shape_is_open = not (Typing_defs.is_nothing s_unknown_value);
    shape_required_fields =
      Typing_defs.TShapeMap.fold
        (fun k { Typing_defs.sft_optional; _ } acc ->
          if not sft_optional then
            k :: acc
          else
            acc)
        s_fields
        [];
    shape_all_fields = Typing_defs.TShapeMap.keys s_fields;
  }

let aggregate_shape_intersection (shapes : shape_info list) : shape_info =
  {
    shape_is_open = List.for_all shapes ~f:(fun s -> s.shape_is_open);
    shape_required_fields =
      List.concat_map shapes ~f:(fun s -> s.shape_required_fields)
      |> List.dedup_and_sort ~compare:Typing_defs.TShapeField.compare;
    shape_all_fields =
      List.concat_map shapes ~f:(fun s -> s.shape_all_fields)
      |> List.dedup_and_sort ~compare:Typing_defs.TShapeField.compare;
  }

let extract_tuple_info
    (t_required : Typing_defs.locl_ty list)
    (t_optional : Typing_defs.locl_ty list)
    (t_extra : Typing_defs.locl_phase Typing_defs.tuple_extra) : tuple_info =
  {
    tuple_arity = Some (List.length t_required + List.length t_optional);
    tuple_has_variadic_tail =
      (match t_extra with
      | Typing_defs.Tvariadic ty -> not (Typing_defs.is_nothing ty)
      | Typing_defs.Tsplat _ -> true);
  }

let aggregate_tuple_intersection (tuples : tuple_info list) : tuple_info =
  let arities = List.filter_map tuples ~f:(fun t -> t.tuple_arity) in
  match arities with
  | [] -> { tuple_arity = None; tuple_has_variadic_tail = false }
  | a :: rest ->
    if List.for_all rest ~f:(fun b -> a = b) then
      {
        tuple_arity = Some a;
        tuple_has_variadic_tail =
          List.for_all tuples ~f:(fun t -> t.tuple_has_variadic_tail);
      }
    else
      { tuple_arity = Some (-1); tuple_has_variadic_tail = false }

let rec walk_shapes
    (env : Tast_env.env)
    (ty : Typing_defs.locl_ty)
    ~(on_shape : shape_info -> unit) : unit =
  let (_, ety) = Tast_env.expand_type env ty in
  match Typing_defs.get_node ety with
  | Tshape { s_fields; s_unknown_value; _ } ->
    on_shape (extract_shape_info s_fields s_unknown_value)
  | Tdynamic _ -> ()
  | Tunion [] -> ()
  | Tunion tyl -> List.iter tyl ~f:(fun ty -> walk_shapes env ty ~on_shape)
  | Tintersection tyl ->
    let member_infos =
      List.map tyl ~f:(fun ty ->
          let infos = ref [] in
          walk_shapes env ty ~on_shape:(fun info -> infos := info :: !infos);
          !infos)
    in
    if List.for_all member_infos ~f:(fun infos -> List.length infos <= 1) then begin
      let shapes = List.filter_map member_infos ~f:List.hd in
      match shapes with
      | [] -> ()
      | shapes -> on_shape (aggregate_shape_intersection shapes)
    end
  | Tnewtype (cid, [ty], _)
    when String.equal cid Naming_special_names.Classes.cSupportDyn ->
    walk_shapes env ty ~on_shape
  | Tgeneric _ ->
    let (_, upper_bounds) =
      Tast_env.get_concrete_supertypes ~abstract_enum:false env ety
    in
    (match upper_bounds with
    | [] -> ()
    | tyl ->
      let inter =
        Typing_make_type.intersection (Typing_defs.get_reason ety) tyl
      in
      walk_shapes env inter ~on_shape)
  | Tnewtype (_, _, bound) -> walk_shapes env bound ~on_shape
  | Tdependent (_, bound) -> walk_shapes env bound ~on_shape
  | Tany _
  | Tnonnull
  | Toption _
  | Tprim _
  | Tfun _
  | Ttuple _
  | Tvec_or_dict _
  | Taccess _
  | Tclass_ptr _
  | Tvar _
  | Tclass _
  | Tneg _
  | Tlabel _ ->
    ()

let rec walk_tuples
    (env : Tast_env.env)
    (ty : Typing_defs.locl_ty)
    ~(on_tuple : tuple_info -> unit) : unit =
  let (_, ety) = Tast_env.expand_type env ty in
  match Typing_defs.get_node ety with
  | Ttuple { t_required; t_optional; t_extra } ->
    on_tuple (extract_tuple_info t_required t_optional t_extra)
  | Tdynamic _ -> ()
  | Tunion [] -> ()
  | Tunion tyl -> List.iter tyl ~f:(fun ty -> walk_tuples env ty ~on_tuple)
  | Tintersection tyl ->
    let member_infos =
      List.map tyl ~f:(fun ty ->
          let infos = ref [] in
          walk_tuples env ty ~on_tuple:(fun info -> infos := info :: !infos);
          !infos)
    in
    if List.for_all member_infos ~f:(fun infos -> List.length infos <= 1) then begin
      let tuples = List.filter_map member_infos ~f:List.hd in
      match tuples with
      | [] -> ()
      | tuples -> on_tuple (aggregate_tuple_intersection tuples)
    end
  | Tnewtype (cid, [ty], _)
    when String.equal cid Naming_special_names.Classes.cSupportDyn ->
    walk_tuples env ty ~on_tuple
  | Tgeneric _ ->
    let (_, upper_bounds) =
      Tast_env.get_concrete_supertypes ~abstract_enum:false env ety
    in
    (match upper_bounds with
    | [] -> ()
    | tyl ->
      let inter =
        Typing_make_type.intersection (Typing_defs.get_reason ety) tyl
      in
      walk_tuples env inter ~on_tuple)
  | Tnewtype (_, _, bound) -> walk_tuples env bound ~on_tuple
  | Tdependent (_, bound) -> walk_tuples env bound ~on_tuple
  | Tany _
  | Tnonnull
  | Toption _
  | Tprim _
  | Tfun _
  | Tshape _
  | Tvec_or_dict _
  | Taccess _
  | Tclass_ptr _
  | Tvar _
  | Tclass _
  | Tneg _
  | Tlabel _ ->
    ()

let field_name_string (sf_name : Typing_defs.TShapeMap.key) : string =
  match sf_name with
  | TSFlit_str (_, s) -> Printf.sprintf "'%s'" s
  | TSFclass_const ((_, cls), (_, mem)) -> Printf.sprintf "%s::%s" cls mem
  | TSFregex_group (_, s) -> Printf.sprintf "/%s/" s

let add_error
    (env : Tast_env.env)
    (error : Typing_error.Primary.Shape_and_tuple_destructure.t) : unit =
  let Equal = Tast_env.eq_typing_env in
  Typing_error_utils.add_typing_error
    ~env
    (Typing_error.primary
       (Typing_error.Primary.Shape_and_tuple_destructure error))

let is_under_dynamic (env : Tast_env.env) : bool =
  let Equal = Tast_env.eq_typing_env in
  Tast.is_under_dynamic_assumptions env.Typing_env_types.checked

(** Example: {[
      $sh : shape('a' => int, ...);
      shape('a' => $a) = $sh;  // error: missing `...`
    ]} *)
let check_missing_ellipsis
    (env : Tast_env.env)
    (shape_pat : Tast.destructure_shape)
    (info : shape_info) : unit =
  if (not shape_pat.ds_ellipsis) && info.shape_is_open then
    add_error env (Missing_ellipsis shape_pat.ds_pos)

(** Example: {[
      $sh : shape('a' => int, 'b' => string);
      shape('a' => $a) = $sh;  // error: missing required field 'b'
    ]} *)
let check_missing_required_fields
    (env : Tast_env.env)
    (shape_pat : Tast.destructure_shape)
    (info : shape_info) : unit =
  if not shape_pat.ds_ellipsis then begin
    let missing_fields =
      List.filter info.shape_required_fields ~f:(fun sf_name ->
          not
            (List.exists shape_pat.ds_fields ~f:(fun shape_field ->
                 Typing_defs.TShapeField.compare
                   (Typing_defs.TShapeField.of_ast
                      Pos_or_decl.of_raw_pos
                      shape_field.dsf_name)
                   sf_name
                 = 0)))
    in
    match missing_fields with
    | [] -> ()
    | missing ->
      let field_names =
        List.map missing ~f:field_name_string |> String.concat ~sep:", "
      in
      add_error
        env
        (Missing_required_fields { pos = shape_pat.ds_pos; field_names })
  end

(** Example: {[
      $sh : shape('a' => int);
      shape('a' => $a, 'z' => $z) = $sh;  // error: field 'z' doesn't exist
    ]} *)
let check_unknown_fields
    (env : Tast_env.env)
    (shape_pat : Tast.destructure_shape)
    (info : shape_info) : unit =
  List.iter shape_pat.ds_fields ~f:(fun shape_field ->
      let sf_name =
        Typing_defs.TShapeField.of_ast
          Pos_or_decl.of_raw_pos
          shape_field.dsf_name
      in
      let field_is_known =
        List.exists info.shape_all_fields ~f:(fun f ->
            Typing_defs.TShapeField.compare f sf_name = 0)
      in
      if not field_is_known then
        let field_pos =
          match shape_field.dsf_name with
          | Ast_defs.SFlit_str (pos, _) -> pos
          | Ast_defs.SFclass_const ((pos, _), _) -> pos
          | Ast_defs.SFclassname (pos, _) -> pos
        in
        add_error
          env
          (Unknown_field
             { pos = field_pos; field_name = field_name_string sf_name }))

let validate_shape_member
    (env : Tast_env.env)
    (shape_pat : Tast.destructure_shape)
    (info : shape_info)
    ~(under_dynamic : bool) : unit =
  if not under_dynamic then begin
    check_missing_ellipsis env shape_pat info;
    check_missing_required_fields env shape_pat info;
    check_unknown_fields env shape_pat info
  end

let check_shape
    (env : Tast_env.env)
    (rhs_ty : Typing_defs.locl_ty)
    (shape_pat : Tast.destructure_shape) : unit =
  let under_dynamic = is_under_dynamic env in
  walk_shapes env rhs_ty ~on_shape:(fun info ->
      validate_shape_member env shape_pat info ~under_dynamic)

let validate_tuple_member
    (env : Tast_env.env)
    (tuple_pat : Tast.destructure_tuple)
    (info : tuple_info) : unit =
  let pat_arity = List.length tuple_pat.dt_entries in
  match info.tuple_arity with
  | Some tuple_arity when tuple_arity >= 0 ->
    if (not tuple_pat.dt_ellipsis) && pat_arity <> tuple_arity then
      add_error
        env
        (Tuple_arity_mismatch { pos = tuple_pat.dt_pos; pat_arity; tuple_arity });
    if
      tuple_pat.dt_ellipsis
      && (not info.tuple_has_variadic_tail)
      && pat_arity > tuple_arity
    then
      add_error
        env
        (Tuple_arity_exceeds { pos = tuple_pat.dt_pos; pat_arity; tuple_arity })
  | Some _ -> add_error env (Tuple_arity_union_conflict tuple_pat.dt_pos)
  | None -> ()

let check_tuple
    (env : Tast_env.env)
    (rhs_ty : Typing_defs.locl_ty)
    (tuple_pat : Tast.destructure_tuple) : unit =
  walk_tuples env rhs_ty ~on_tuple:(fun info ->
      validate_tuple_member env tuple_pat info)

let handler : Tast_visitor.handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr (env : Tast_env.env) ((ty, _pos, expr_) : Tast.expr) : unit
        =
      match expr_ with
      | DestructureShape shape_pat -> check_shape env ty shape_pat
      | DestructureTuple tuple_pat -> check_tuple env ty tuple_pat
      | _ -> ()
  end
