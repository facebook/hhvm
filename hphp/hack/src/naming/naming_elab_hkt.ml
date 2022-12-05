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

let on_hint (env, hint, err_acc) =
  match hint with
  | (pos, Aast.Habstr (name, hints)) ->
    let err =
      match hints with
      | [] -> err_acc
      | _ ->
        Err.Free_monoid.plus
          err_acc
          (Err.naming
          @@ Naming_error.Tparam_applied_to_type { pos; tparam_name = name })
    in
    Naming_phase_pass.Cont.next (env, (pos, Aast.Habstr (name, [])), err)
  | _ -> Naming_phase_pass.Cont.next (env, hint, err_acc)

let on_tparam
    ( env,
      (Aast.{ tp_parameters; tp_name = (pos, tparam_name); _ } as tparam),
      err_acc ) =
  match tp_parameters with
  | [] -> Naming_phase_pass.Cont.next (env, tparam, err_acc)
  | _ ->
    let err =
      Err.Free_monoid.plus err_acc
      @@ Err.naming
      @@ Naming_error.Tparam_with_tparam { pos; tparam_name }
    in
    Naming_phase_pass.Cont.next
      (env, Aast.{ tparam with tp_parameters = [] }, err)

let pass =
  Naming_phase_pass.(
    top_down
      { identity with on_hint = Some on_hint; on_tparam = Some on_tparam })

let visitor = Naming_phase_pass.mk_visitor [pass]

let elab f ?init ?(env = Env.empty) elem =
  Tuple2.map_snd ~f:(Err.from_monoid ?init) @@ f env elem

let elab_fun_def ?init ?env elem = elab visitor#on_fun_def ?init ?env elem

let elab_typedef ?init ?env elem = elab visitor#on_typedef ?init ?env elem

let elab_module_def ?init ?env elem = elab visitor#on_module_def ?init ?env elem

let elab_gconst ?init ?env elem = elab visitor#on_gconst ?init ?env elem

let elab_class ?init ?env elem = elab visitor#on_class_ ?init ?env elem

let elab_program ?init ?env elem = elab visitor#on_program ?init ?env elem
