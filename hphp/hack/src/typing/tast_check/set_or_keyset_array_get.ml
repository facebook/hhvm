(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
open Typing_defs

let warning_kind = Typing_warning.Set_or_keyset_array_get

let error_codes = Typing_warning_utils.codes warning_kind

let is_keyset env ty =
  let r = Typing_reason.none in
  Tast_env.is_sub_type
    env
    ty
    (Typing_make_type.keyset r (Typing_make_type.arraykey r))

let is_nothing env ty =
  let r = Typing_reason.none in
  Tast_env.is_sub_type env ty (Typing_make_type.nothing r)

let is_const_set env ty =
  let r = Typing_reason.none in
  Tast_env.is_sub_type
    env
    ty
    (Typing_make_type.class_type
       r
       Naming_special_names.Collections.cConstSet
       [Typing_make_type.arraykey r])

let get_text_at_pos (source_text : Full_fidelity_source_text.t) (pos : Pos.t) :
    string option =
  let (start_offset, end_offset) = Pos.info_raw pos in
  if start_offset >= 0 && end_offset > start_offset then
    Some
      (Full_fidelity_source_text.sub
         source_text
         start_offset
         (end_offset - start_offset))
  else
    None

let compute_replacement_text
    env
    (kind : Typing_warning.Set_or_keyset_array_get.kind)
    (arr_pos : Pos.t)
    (idx_pos : Pos.t) : string option =
  let open Option.Let_syntax in
  let ctx = Tast_env.get_ctx env in
  let file = Tast_env.get_file env in
  let entries = Provider_context.get_entries ctx in
  let* entry = Relative_path.Map.find_opt entries file in
  let* source_text = entry.Provider_context.source_text in
  let* arr_text = get_text_at_pos source_text arr_pos in
  let+ idx_text = get_text_at_pos source_text idx_pos in
  let replacement =
    match kind with
    | Typing_warning.Set_or_keyset_array_get.Keyset ->
      Printf.sprintf "C\\contains_key(%s, %s)" arr_text idx_text
    | Typing_warning.Set_or_keyset_array_get.ConstSet ->
      Printf.sprintf "%s->contains(%s)" arr_text idx_text
  in
  replacement

let rec check_array_get env pos full_expr_pos arr_pos idx_pos ty =
  let (env, expanded_ty) = Tast_env.expand_type env ty in
  let expanded_ty = Tast_env.strip_dynamic env expanded_ty in
  match get_node expanded_ty with
  | Tunion tyl ->
    List.iter tyl ~f:(check_array_get env pos full_expr_pos arr_pos idx_pos)
  | Toption inner_ty ->
    check_array_get env pos full_expr_pos arr_pos idx_pos inner_ty
  | _ ->
    (* Use subtype checking to detect keyset or ConstSet subtypes *)
    let kind_opt =
      if not (is_nothing env expanded_ty) then
        if is_keyset env expanded_ty then
          Some Typing_warning.Set_or_keyset_array_get.Keyset
        else if is_const_set env expanded_ty then
          Some Typing_warning.Set_or_keyset_array_get.ConstSet
        else
          None
      else
        None
    in
    (match kind_opt with
    | Some kind ->
      let replacement_text =
        compute_replacement_text env kind arr_pos idx_pos
      in
      Tast_env.add_warning
        env
        ( pos,
          Typing_warning.Set_or_keyset_array_get,
          {
            Typing_warning.Set_or_keyset_array_get.kind;
            ty = Tast_env.print_ty env expanded_ty;
            full_expr_pos;
            replacement_text;
          } )
    | None -> ())

[@@@warning "-27"]

(* Use a stateful visitor to track when we're inside an unset() call.
 * We skip the warning for Array_get inside unset() since that's a valid
 * way to remove elements from a keyset/set.
 *)
let visitor =
  object (this)
    inherit [_] Tast_visitor.iter_with_state as super

    method! on_expr (env, in_unset_or_assign) ((_, p, expr) as e) =
      match expr with
      (* Detect unset($arr[key]) - mark children as in_unset context *)
      | Call { func = (_, _, Id (_, name)); args; _ }
        when String.equal name Naming_special_names.PseudoFunctions.unset
             || String.equal name Naming_special_names.PseudoFunctions.isset ->
        List.iter args ~f:(fun arg ->
            this#on_expr (env, true) (Aast_utils.arg_to_expr arg))
      | Assign (e1, _op, e2) ->
        this#on_expr (env, true) e1;
        this#on_expr (env, false) e2
      (* Check Array_get only when NOT in unset context *)
      | Array_get (((ty, arr_p, _) as arr), Some ((_, idx_p, _) as idx)) ->
        if not in_unset_or_assign then check_array_get env p p arr_p idx_p ty;
        this#on_expr (env, false) arr;
        this#on_expr (env, false) idx
      | _ -> super#on_expr (env, false) e
  end

let handler ~as_lint =
  object
    inherit Tast_visitor.handler_base

    method! at_fun_def env = visitor#on_fun_def (env, false)

    method! at_method_ env = visitor#on_method_ (env, false)
  end
