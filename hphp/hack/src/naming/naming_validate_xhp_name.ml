(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Err = Naming_phase_error

let on_hint_ (env, hint_, err) =
  let err =
    match hint_ with
    (* some common Xhp screw ups *)
    | Aast.Happly ((pos, ty_name), _hints)
      when String.(
             equal ty_name "Xhp" || equal ty_name ":Xhp" || equal ty_name "XHP")
      ->
      (Err.naming @@ Naming_error.Disallowed_xhp_type { pos; ty_name }) :: err
    | _ -> err
  in
  Ok (env, hint_, err)

let pass =
  Naming_phase_pass.(
    top_down Ast_transform.{ identity with on_hint_ = Some on_hint_ })
