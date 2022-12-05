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

let elab_value = function
  | (annot, pos, Aast.Id _) ->
    let err = Err.naming @@ Naming_error.Expected_variable pos in
    let ident = Local_id.make_unscoped "__internal_placeholder" in
    ((annot, pos, Aast.Lvar (pos, ident)), Some err)
  | expr -> (expr, None)

let elab_key = function
  | (_, _, Aast.(Lvar _ | Lplaceholder _)) as expr -> (expr, None)
  | (annot, pos, _) ->
    let err = Err.naming @@ Naming_error.Expected_variable pos in
    let ident = Local_id.make_unscoped "__internal_placeholder" in
    ((annot, pos, Aast.Lvar (pos, ident)), Some err)

let on_as_expr (env, as_expr, err_acc) =
  let open Naming_phase_pass.Cont in
  let (as_expr, err_opt) =
    match as_expr with
    | Aast.As_v e ->
      let (e, err) = elab_value e in
      (Aast.As_v e, err)
    | Aast.Await_as_v (pos, e) ->
      let (e, err) = elab_value e in
      (Aast.Await_as_v (pos, e), err)
    | Aast.As_kv (ke, ve) ->
      let (ke, kerr) = elab_key ke and (ve, verr) = elab_value ve in
      (Aast.As_kv (ke, ve), Option.merge ~f:Err.Free_monoid.plus kerr verr)
    | Aast.Await_as_kv (pos, ke, ve) ->
      let (ke, kerr) = elab_key ke and (ve, verr) = elab_value ve in
      ( Aast.Await_as_kv (pos, ke, ve),
        Option.merge ~f:Err.Free_monoid.plus kerr verr )
  in
  let err =
    Option.value_map err_opt ~default:err_acc ~f:(Err.Free_monoid.plus err_acc)
  in
  next (env, as_expr, err)

let pass =
  Naming_phase_pass.(top_down { identity with on_as_expr = Some on_as_expr })

let visitor = Naming_phase_pass.mk_visitor [pass]

let elab f ?init ?(env = Env.empty) elem =
  Tuple2.map_snd ~f:(Err.from_monoid ?init) @@ f env elem

let elab_fun_def ?init ?env elem = elab visitor#on_fun_def ?init ?env elem

let elab_typedef ?init ?env elem = elab visitor#on_typedef ?init ?env elem

let elab_module_def ?init ?env elem = elab visitor#on_module_def ?init ?env elem

let elab_gconst ?init ?env elem = elab visitor#on_gconst ?init ?env elem

let elab_class ?init ?env elem = elab visitor#on_class_ ?init ?env elem

let elab_program ?init ?env elem = elab visitor#on_program ?init ?env elem
