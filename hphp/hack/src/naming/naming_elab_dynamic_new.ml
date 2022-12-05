(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Err = Naming_phase_error
module SN = Naming_special_names

module Env = struct
  type t = unit

  let empty = ()
end

let visitor =
  object (self)
    inherit [_] Aast_defs.mapreduce as super

    inherit Err.monoid

    method! on_New env ((_, pos, ci) as class_id) targs exprs expr_opt ex =
      let (class_id, err) =
        match ci with
        | Aast.CIparent
        | Aast.CIself
        | Aast.CIstatic
        | Aast.CI _
        | Aast.(CIexpr (_, _, (This | Lvar _))) ->
          (class_id, self#zero)
        | Aast.CIexpr (_, ci_pos, _e) ->
          let err = Err.naming @@ Naming_error.Dynamic_new_in_strict_mode pos in
          let class_id = ((), pos, Aast.CI (ci_pos, SN.Classes.cUnknown)) in
          (class_id, err)
      in
      let (expr, err_super) =
        super#on_New env class_id targs exprs expr_opt ex
      in
      (expr, self#plus err_super err)
  end

let elab f ?init ?(env = Env.empty) elem =
  Tuple2.map_snd ~f:(Err.from_monoid ?init) @@ f env elem

let elab_fun_def ?init ?env elem = elab visitor#on_fun_def ?init ?env elem

let elab_typedef ?init ?env elem = elab visitor#on_typedef ?init ?env elem

let elab_module_def ?init ?env elem = elab visitor#on_module_def ?init ?env elem

let elab_gconst ?init ?env elem = elab visitor#on_gconst ?init ?env elem

let elab_class ?init ?env elem = elab visitor#on_class_ ?init ?env elem

let elab_program ?init ?env elem = elab visitor#on_program ?init ?env elem
