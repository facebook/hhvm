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

let is_generated pos =
  String.is_substring
    ~substring:"generated"
    (Pos.to_relative_string pos |> Pos.filename)

let create_handler _ctx =
  object
    inherit Tast_visitor.handler_base

    method! at_method_ env Aast.{ m_ret = (ty, hint_opt); _ } =
      match hint_opt with
      | Some (pos, _) when not @@ is_generated pos ->
        let rec is_toplevelish_dynamic ty =
          let (_env, ty) = Tast_env.expand_type env ty in
          match T.get_node ty with
          | T.Tdynamic -> true
          | T.Toption ty -> is_toplevelish_dynamic ty
          | T.Tclass ((_, id), _, [ty])
            when String.equal id SN.Classes.cAwaitable ->
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
        if is_toplevelish_dynamic ty then log pos EXPLICIT_DYNAMIC
      | _ -> ()

    method! at_expr _env (_, pos, expr) =
      if not @@ is_generated pos then
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
              {
                func = (_, _, Aast.Class_const ((_, _, Aast.CI (_, cid)), _));
                _;
              }
          when String.equal cid nPHPism_FIXME ->
          log PHPISM
        | _ -> ()
  end
