(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

module Env : sig
  type t

  val empty : t
end = struct
  type t = unit

  let empty = ()
end

let rec concat_blocks stmts =
  match stmts with
  | [] -> []
  | (_, Aast.Block b) :: rest -> concat_blocks (b @ rest)
  | next :: rest ->
    let rest = concat_blocks rest in
    next :: rest

let on_block (env, stmts, err) =
  Naming_phase_pass.Cont.next (env, concat_blocks stmts, err)

let on_using_stmt (env, us, err) =
  Naming_phase_pass.Cont.next
    (env, Aast.{ us with us_is_block_scoped = false }, err)

let pass =
  Naming_phase_pass.(
    top_down
      {
        identity with
        on_block = Some on_block;
        on_using_stmt = Some on_using_stmt;
      })

let visitor = Naming_phase_pass.mk_visitor [pass]

let elab f ?(env = Env.empty) elem = fst @@ f env elem

let elab_fun_def ?env elem = elab visitor#on_fun_def ?env elem

let elab_typedef ?env elem = elab visitor#on_typedef ?env elem

let elab_module_def ?env elem = elab visitor#on_module_def ?env elem

let elab_gconst ?env elem = elab visitor#on_gconst ?env elem

let elab_class ?env elem = elab visitor#on_class_ ?env elem

let elab_program ?env elem = elab visitor#on_program ?env elem
