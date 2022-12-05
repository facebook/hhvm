(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Env = struct
  type t = unit

  let empty = ()
end

let visitor =
  object (_self)
    inherit [_] Aast_defs.endo as super

    method on_'ex _ ex = ex

    method on_'en _ en = en

    method! on_expr env ((annot, pos, expr_) as expr) =
      match expr_ with
      | Aast.Import _ -> (annot, pos, Naming_phase_error.invalid_expr_ pos)
      | _ -> super#on_expr env expr
  end

let elab f ?(env = Env.empty) elem = f env elem

let elab_fun_def ?env elem = elab visitor#on_fun_def ?env elem

let elab_typedef ?env elem = elab visitor#on_typedef ?env elem

let elab_module_def ?env elem = elab visitor#on_module_def ?env elem

let elab_gconst ?env elem = elab visitor#on_gconst ?env elem

let elab_class ?env elem = elab visitor#on_class_ ?env elem

let elab_program ?env elem = elab visitor#on_program ?env elem
