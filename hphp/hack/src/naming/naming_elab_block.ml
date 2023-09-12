(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let rec concat_blocks stmts =
  match stmts with
  | [] -> []
  | (_, Aast.Block (_, b)) :: rest -> concat_blocks (b @ rest)
  | next :: rest ->
    let rest = concat_blocks rest in
    next :: rest

let on_block stmts ~ctx = (ctx, Ok (concat_blocks stmts))

let on_using_stmt us ~ctx = (ctx, Ok Aast.{ us with us_is_block_scoped = false })

let pass =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.top_down
    Aast.Pass.
      {
        id with
        on_ty_block = Some on_block;
        on_ty_using_stmt = Some on_using_stmt;
      }
