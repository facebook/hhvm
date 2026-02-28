(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Err = Naming_phase_error

let on_hint_ on_error hint_ ~ctx =
  let err_opt =
    match hint_ with
    (* some common Xhp screw ups *)
    | Aast.Happly ((pos, ty_name), _hints)
      when String.(
             equal ty_name "Xhp" || equal ty_name ":Xhp" || equal ty_name "XHP")
      ->
      Some (Err.naming @@ Naming_error.Disallowed_xhp_type { pos; ty_name })
    | _ -> None
  in
  Option.iter ~f:on_error err_opt;
  (ctx, Ok hint_)

let pass on_error =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.top_down
    Aast.Pass.{ id with on_ty_hint_ = Some (on_hint_ on_error) }
