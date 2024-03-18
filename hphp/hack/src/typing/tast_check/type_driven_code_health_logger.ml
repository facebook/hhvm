(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module SN = Naming_special_names
module T = Typing_defs

type kind =
  | HH_FIXME
  | UNSAFE_CAST
  | PHPISM
  | EXPLICIT_DYNAMIC
  | TYPE_ASSERTION
[@@deriving show { with_path = false }]

let nPHPism_FIXME = "\\PHPism_FIXME"

let log pos kind =
  let json =
    Hh_json.JSON_Object
      [
        ("kind", Hh_json.JSON_String (show_kind kind));
        ("pos", Pos.json (Pos.to_relative_string pos));
      ]
  in
  Hh_logger.log "[TDCH] %s" (Hh_json.json_to_string json)

let is_toplevelish_dynamic env (ty, hint_opt) =
  match hint_opt with
  | Some (pos, _) ->
    let rec is_toplevelish_dynamic ty =
      let (_env, ty) = Tast_env.expand_type env ty in
      let ty =
        Typing_utils.strip_dynamic (Tast_env.tast_env_as_typing_env env) ty
      in
      let (_env, ty) = Tast_env.expand_type env ty in
      match T.get_node ty with
      | T.Tdynamic -> true
      | T.Toption ty -> is_toplevelish_dynamic ty
      | T.Tclass ((_, id), _, [ty]) when String.equal id SN.Classes.cAwaitable
        ->
        is_toplevelish_dynamic ty
      | T.Tclass ((_, id), _, [ty])
        when List.mem
               ~equal:String.equal
               [SN.Collections.cVec; SN.Collections.cVector]
               id ->
        is_toplevelish_dynamic ty
      | T.Tclass ((_, id), _, [_; ty])
        when List.mem
               ~equal:String.equal
               [SN.Collections.cDict; SN.Collections.cMap]
               id ->
        is_toplevelish_dynamic ty
      | _ -> false
    in
    if is_toplevelish_dynamic ty then
      Some pos
    else
      None
  | _ -> None

let log_explicit_dynamic env hint =
  match is_toplevelish_dynamic env hint with
  | Some pos -> log pos EXPLICIT_DYNAMIC
  | None -> ()

let log_callable_def env ret params =
  log_explicit_dynamic env ret;
  List.iter params ~f:(fun param ->
      log_explicit_dynamic env param.Aast.param_type_hint)

let create_handler _ctx =
  object
    inherit Tast_visitor.handler_base

    method! at_fun_ env Aast.{ f_ret; f_params; _ } =
      log_callable_def env f_ret f_params

    method! at_method_ env Aast.{ m_ret; m_params; _ } =
      log_callable_def env m_ret m_params

    method! at_expr _env (_, pos, expr) =
      let log = log pos in
      match expr with
      | Aast.Hole (_, _, _, kind) -> begin
        match kind with
        | Aast.Typing -> log HH_FIXME
        | Aast.UnsafeCast _ -> log UNSAFE_CAST
        | Aast.UnsafeNonnullCast -> log UNSAFE_CAST
        | _ -> ()
      end
      | Aast.As _ -> log TYPE_ASSERTION
      | Aast.Call
          Aast.
            { func = (_, _, Aast.Class_const ((_, _, Aast.CI (_, cid)), _)); _ }
        when String.equal cid nPHPism_FIXME ->
        log PHPISM
      | _ -> ()
  end
