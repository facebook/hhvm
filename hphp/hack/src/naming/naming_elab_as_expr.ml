(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Err = Naming_phase_error

module Env = struct
  type t = unit

  let empty = ()
end

let visitor =
  object (self)
    inherit [_] Aast_defs.mapreduce as super

    inherit Err.monoid

    method! on_as_expr env as_expr =
      match as_expr with
      | Aast.As_v e ->
        let (e, err) = self#elab_value env e in
        (Aast.As_v e, err)
      | Aast.Await_as_v (pos, e) ->
        let (e, err) = self#elab_value env e in
        (Aast.Await_as_v (pos, e), err)
      | Aast.As_kv (ke, ve) ->
        let (ke, kerr) = self#elab_key env ke
        and (ve, verr) = self#elab_value env ve in
        (Aast.As_kv (ke, ve), self#plus kerr verr)
      | Aast.Await_as_kv (pos, ke, ve) ->
        let (ke, kerr) = self#elab_key env ke
        and (ve, verr) = self#elab_value env ve in
        (Aast.Await_as_kv (pos, ke, ve), self#plus kerr verr)

    method private elab_value env ((annot, pos, expr_) as expr) =
      match expr_ with
      | Aast.Id _ ->
        let err = Err.naming @@ Naming_error.Expected_variable pos in
        let ident = Local_id.make_unscoped "__internal_placeholder" in
        ((annot, pos, Aast.Lvar (pos, ident)), err)
      | _ -> super#on_expr env expr

    method private elab_key _env ((annot, pos, expr_) as expr) =
      match expr_ with
      | Aast.(Lvar _ | Lplaceholder _) -> (expr, self#zero)
      | _ ->
        let err = Err.naming @@ Naming_error.Expected_variable pos in
        let ident = Local_id.make_unscoped "__internal_placeholder" in
        ((annot, pos, Aast.Lvar (pos, ident)), err)
  end

let elab f ?init ?(env = Env.empty) elem =
  Tuple2.map_snd ~f:(Err.from_monoid ?init) @@ f env elem

let elab_fun_def ?init ?env elem = elab visitor#on_fun_def ?init ?env elem

let elab_typedef ?init ?env elem = elab visitor#on_typedef ?init ?env elem

let elab_module_def ?init ?env elem = elab visitor#on_module_def ?init ?env elem

let elab_gconst ?init ?env elem = elab visitor#on_gconst ?init ?env elem

let elab_class ?init ?env elem = elab visitor#on_class_ ?init ?env elem

let elab_program ?init ?env elem = elab visitor#on_program ?init ?env elem
