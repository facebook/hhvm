(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let on_expr_ expr_ ~ctx =
  match expr_ with
  | Aast.Invalid _ -> (ctx, Error expr_)
  | _ -> (ctx, Ok expr_)

let pass =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.top_down Aast.Pass.{ id with on_ty_expr_ = Some on_expr_ }
