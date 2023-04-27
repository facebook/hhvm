(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Err = Naming_phase_error

let on_expr on_error expr ~ctx =
  match expr with
  | (_, pos, Aast.Tuple []) ->
    on_error (Err.naming @@ Naming_error.Too_few_arguments pos);
    (ctx, Error (Err.invalid_expr expr))
  | _ -> (ctx, Ok expr)

let pass on_error =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.top_down
    Aast.Pass.
      {
        id with
        on_ty_expr = Some (fun elem ~ctx -> on_expr on_error elem ~ctx);
      }
