(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let on_expr_ on_error expr_ ~ctx =
  let err_opt =
    match expr_ with
    | Aast.(Is (_, (pos, Hdynamic))) ->
      Some
        (Naming_phase_error.naming @@ Naming_error.Dynamic_hint_disallowed pos)
    | _ -> None
  in
  Option.iter ~f:on_error err_opt;
  (ctx, Ok expr_)

let pass on_error =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.bottom_up
    Aast.Pass.
      {
        id with
        on_ty_expr_ = Some (fun elem ~ctx -> on_expr_ on_error elem ~ctx);
      }
