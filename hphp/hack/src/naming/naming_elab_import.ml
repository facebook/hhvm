(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let on_expr expr ~ctx =
  match expr with
  | (_, _, Aast.Import _) -> (ctx, Error (Naming_phase_error.invalid_expr expr))
  | _ -> (ctx, Ok expr)

let pass =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.top_down Aast.Pass.{ id with on_ty_expr = Some on_expr }
