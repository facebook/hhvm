(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Env : sig
  type t

  val empty : t
end = struct
  type t = unit

  let empty = ()
end

let on_expr (env, expr, err_acc) =
  match expr with
  | (annot, pos, Aast.Import _) ->
    Naming_phase_pass.Cont.finish
      (env, (annot, pos, Naming_phase_error.invalid_expr_ pos), err_acc)
  | _ -> Naming_phase_pass.Cont.next (env, expr, err_acc)

let pass = Naming_phase_pass.(top_down { identity with on_expr = Some on_expr })

let visitor = Naming_phase_pass.mk_visitor [pass]

let elab f ?(env = Env.empty) elem = fst @@ f env elem

let elab_fun_def ?env elem = elab visitor#on_fun_def ?env elem

let elab_typedef ?env elem = elab visitor#on_typedef ?env elem

let elab_module_def ?env elem = elab visitor#on_module_def ?env elem

let elab_gconst ?env elem = elab visitor#on_gconst ?env elem

let elab_class ?env elem = elab visitor#on_class_ ?env elem

let elab_program ?env elem = elab visitor#on_program ?env elem
