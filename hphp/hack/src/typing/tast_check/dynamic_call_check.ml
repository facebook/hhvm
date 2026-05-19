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

let warning_kind = Typing_warning.Dynamic_call

let error_codes = Typing_warning_utils.codes warning_kind

let rec is_dynamic_or_intersects_dynamic env ty =
  let (env, ty) = Tast_env.expand_type env ty in
  match get_node ty with
  | Tdynamic _ -> Some (env, ty)
  | Tintersection tyl ->
    List.find_map tyl ~f:(is_dynamic_or_intersects_dynamic env)
  | Tnewtype (name, [tyarg], _)
    when String.equal name Naming_special_names.Classes.cSupportDyn ->
    is_dynamic_or_intersects_dynamic env tyarg
  | _ -> None

let rec strip_supportdyn env ty =
  let (env, ty) = Tast_env.expand_type env ty in
  match get_node ty with
  | Tnewtype (name, [tyarg], _)
    when String.equal name Naming_special_names.Classes.cSupportDyn ->
    strip_supportdyn env tyarg
  | _ -> (env, ty)

let check_dynamic_call env pos kind ty =
  match is_dynamic_or_intersects_dynamic env ty with
  | Some (_env, dynamic_ty) ->
    let file = Tast_env.get_file env in
    let path = Relative_path.suffix file in
    (* Don't bother with tests as dynamic is too prolific there *)
    if not (String.is_substring path ~substring:"/__tests__/") then begin
      let (_env, ty_for_print) = strip_supportdyn env ty in
      let actual_type = Tast_env.print_ty env ty_for_print in
      let r = get_reason dynamic_ty in
      let (dynamic_reason_pos, dynamic_reason_msg) =
        match
          Typing_reason.to_string "This expression has type `dynamic`" r
        with
        | (p, msg) :: _ -> (p, msg)
        | [] ->
          (Pos_or_decl.of_raw_pos pos, "This expression has type `dynamic`")
      in
      Tast_env.add_warning
        env
        ( pos,
          Typing_warning.Dynamic_call,
          {
            Typing_warning.Dynamic_call.kind;
            actual_type;
            dynamic_reason_pos;
            dynamic_reason_msg;
          } )
    end
  | None -> ()

let handler ~as_lint:_ =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env (_, _, expr_) =
      match expr_ with
      | Call
          {
            func =
              ( _,
                _,
                Obj_get
                  ((receiver_ty, _, _), (_, method_pos, _), _, Aast.Is_method)
              );
            _;
          } ->
        check_dynamic_call
          env
          method_pos
          Typing_warning.Dynamic_call.Method_invocation
          receiver_ty
      | Obj_get ((receiver_ty, _, _), (_, prop_pos, _), _, Aast.Is_prop) ->
        check_dynamic_call
          env
          prop_pos
          Typing_warning.Dynamic_call.Property_access
          receiver_ty
      | Call { func = (fty, fpos, _); _ } ->
        check_dynamic_call
          env
          fpos
          Typing_warning.Dynamic_call.Function_call
          fty
      | Array_get ((base_ty, base_pos, _), None) ->
        check_dynamic_call
          env
          base_pos
          Typing_warning.Dynamic_call.Array_append
          base_ty
      | Array_get ((base_ty, base_pos, _), Some _) ->
        check_dynamic_call
          env
          base_pos
          Typing_warning.Dynamic_call.Array_index
          base_ty
      | _ -> ()
  end
