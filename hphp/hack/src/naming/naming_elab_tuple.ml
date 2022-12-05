(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Err = Naming_phase_error

module Env : sig
  type t

  val empty : t
end = struct
  type t = unit

  let empty = ()
end

let on_expr (env, expr, err_acc) =
  let res =
    match expr with
    | (annot, pos, Aast.Tuple []) ->
      let err = Err.naming @@ Naming_error.Too_few_arguments pos in
      Error ((annot, pos, Err.invalid_expr_ pos), err)
    | _ -> Ok expr
  in
  match res with
  | Ok expr -> Naming_phase_pass.Cont.next (env, expr, err_acc)
  | Error (expr, err) ->
    Naming_phase_pass.Cont.finish (env, expr, Err.Free_monoid.plus err_acc err)

let pass = Naming_phase_pass.(top_down { identity with on_expr = Some on_expr })

let visitor = Naming_phase_pass.mk_visitor [pass]

let elab f ?init ?(env = Env.empty) elem =
  Tuple2.map_snd ~f:(Err.from_monoid ?init) @@ f env elem

let elab_fun_def ?init ?env elem = elab visitor#on_fun_def ?init ?env elem

let elab_typedef ?init ?env elem = elab visitor#on_typedef ?init ?env elem

let elab_module_def ?init ?env elem = elab visitor#on_module_def ?init ?env elem

let elab_gconst ?init ?env elem = elab visitor#on_gconst ?init ?env elem

let elab_class ?init ?env elem = elab visitor#on_class_ ?init ?env elem

let elab_program ?init ?env elem = elab visitor#on_program ?init ?env elem
