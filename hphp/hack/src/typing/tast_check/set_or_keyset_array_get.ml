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

let rec check_array_get env pos ty =
  let (env, expanded_ty) = Tast_env.expand_type env ty in
  let expanded_ty = Tast_env.strip_dynamic env expanded_ty in
  match get_node expanded_ty with
  | Tunion tyl -> List.iter tyl ~f:(check_array_get env pos)
  | Toption inner_ty -> check_array_get env pos inner_ty
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
      Tast_env.add_warning
        env
        ( pos,
          Typing_warning.Set_or_keyset_array_get,
          {
            Typing_warning.Set_or_keyset_array_get.kind;
            ty = Tast_env.print_ty env expanded_ty;
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
      | Array_get (((ty, _, _) as arr), Some idx) ->
        if not in_unset_or_assign then check_array_get env p ty;
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
