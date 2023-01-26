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
  | (_, Aast.Block b) :: rest -> concat_blocks (b @ rest)
  | next :: rest ->
    let rest = concat_blocks rest in
    next :: rest

let on_block (env, stmts, err) = Ok (env, concat_blocks stmts, err)

let on_using_stmt :
      'a 'b.
      'env * ('a, 'b) Aast_defs.using_stmt * _ ->
      ('env * ('a, 'b) Aast_defs.using_stmt * _, _) result =
 fun (env, us, err) ->
  Ok (env, Aast.{ us with us_is_block_scoped = false }, err)

let pass =
  Naming_phase_pass.(
    top_down
      Ast_transform.
        {
          identity with
          on_block = Some on_block;
          on_using_stmt = Some on_using_stmt;
        })
