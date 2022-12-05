(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

module Env = struct
  type t = bool

  let empty = false
end

let visitor =
  object (self)
    inherit [_] Aast_defs.endo as super

    method on_'ex _ ex = ex

    method on_'en _ en = en

    method! on_block env stmts =
      let stmts = self#concat_blocks env stmts in
      super#on_block env stmts

    (* TODO[mjt] I don't think this is actually necessary? *)
    method! on_using_stmt env us =
      super#on_using_stmt env Aast.{ us with us_is_block_scoped = false }

    method private concat_blocks env stmts =
      match stmts with
      | [] -> []
      | (_, Aast.Block b) :: rest -> self#concat_blocks env (b @ rest)
      | next :: rest ->
        let rest = self#concat_blocks env rest in
        next :: rest
  end

let elab f ?(env = Env.empty) elem = f env elem

let elab_fun_def ?env elem = elab visitor#on_fun_def ?env elem

let elab_typedef ?env elem = elab visitor#on_typedef ?env elem

let elab_module_def ?env elem = elab visitor#on_module_def ?env elem

let elab_gconst ?env elem = elab visitor#on_gconst ?env elem

let elab_class ?env elem = elab visitor#on_class_ ?env elem

let elab_program ?env elem = elab visitor#on_program ?env elem
