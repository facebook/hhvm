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

  val soft_as_like : t -> bool

  val set_soft_as_like : t -> soft_as_like:bool -> t
end = struct
  type t = { soft_as_like: bool }

  let empty = { soft_as_like = false }

  let soft_as_like { soft_as_like } = soft_as_like

  let set_soft_as_like _ ~soft_as_like = { soft_as_like }
end

let on_hint (env, hint, err) =
  let hint =
    match hint with
    | (pos, Aast.Hsoft hint) when Env.soft_as_like env -> (pos, Aast.Hlike hint)
    | (pos, Aast.Hsoft (_, hint_)) -> (pos, hint_)
    | _ -> hint
  in
  Naming_phase_pass.Cont.next (env, hint, err)

let pass = Naming_phase_pass.(top_down { identity with on_hint = Some on_hint })

let visitor = Naming_phase_pass.mk_visitor [pass]

let elab f ?(env = Env.empty) elem = fst @@ f env elem

let elab_fun_def ?env elem = elab visitor#on_fun_def ?env elem

let elab_typedef ?env elem = elab visitor#on_typedef ?env elem

let elab_module_def ?env elem = elab visitor#on_module_def ?env elem

let elab_gconst ?env elem = elab visitor#on_gconst ?env elem

let elab_class ?env elem = elab visitor#on_class_ ?env elem

let elab_program ?env elem = elab visitor#on_program ?env elem
