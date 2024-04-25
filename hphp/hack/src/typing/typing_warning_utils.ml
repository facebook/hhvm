(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core
module Codes = Error_codes.Warning

type t = {
  code: Codes.t;
  claim: Pos.t Message.t Lazy.t;
  reasons: Pos_or_decl.t Message.t list Lazy.t;
  quickfixes: Pos.t Quickfix.t list;
}

let severity = User_error.Warning

let to_user_error (warning : t) : Errors.error =
  let { code; claim; reasons; quickfixes } = warning in
  {
    User_error.severity;
    code = Codes.to_enum code;
    claim = Lazy.force claim;
    reasons = Lazy.force reasons;
    quickfixes;
    flags = User_error_flags.empty;
    custom_msgs = [];
    is_fixmed = false;
  }

let sketchy_equality pos b left right left_trail right_trail =
  let typedef_trail_entry pos = (pos, "Typedef definition comes from here") in
  let claim =
    lazy
      (let b =
         Markdown_lite.md_codify
           (if b then
             "true"
           else
             "false")
       in
       let msg = Printf.sprintf "This expression is likely always %s" b in
       (pos, msg))
  and reasons =
    Lazy.(
      left >>= fun left ->
      right >>= fun right ->
      let left_trail = List.map left_trail ~f:typedef_trail_entry in
      let right_trail = List.map right_trail ~f:typedef_trail_entry in
      return (left @ left_trail @ right @ right_trail))
  in
  { code = Codes.SketchyEquality; claim; reasons; quickfixes = [] }

let to_error (warning : Typing_warning.t) : t =
  let open Typing_warning in
  match warning with
  | Sketchy_equality { pos; result; left; right; left_trail; right_trail } ->
    sketchy_equality pos result left right left_trail right_trail

let add (env : Typing_env_types.env) (warning : Typing_warning.t) : unit =
  if TypecheckerOptions.hack_warnings (Typing_env.get_tcopt env) then
    Errors.add_error @@ to_user_error @@ to_error warning
