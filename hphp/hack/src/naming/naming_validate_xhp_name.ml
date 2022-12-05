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

let on_hint_ (env, hint_, err) =
  let err =
    match hint_ with
    (* some common Xhp screw ups *)
    | Aast.Happly ((pos, ty_name), _hints)
      when String.(
             equal ty_name "Xhp" || equal ty_name ":Xhp" || equal ty_name "XHP")
      ->
      Err.Free_monoid.plus err
      @@ Err.naming
      @@ Naming_error.Disallowed_xhp_type { pos; ty_name }
    | _ -> err
  in
  Naming_phase_pass.Cont.next (env, hint_, err)

let pass =
  Naming_phase_pass.(top_down { identity with on_hint_ = Some on_hint_ })

let visitor = Naming_phase_pass.mk_visitor [pass]

let validate f ?init ?(env = Env.empty) elem =
  Err.from_monoid ?init @@ snd @@ f env elem

let validate_program ?init ?env elem =
  validate visitor#on_program ?init ?env elem

let validate_module_def ?init ?env elem =
  validate visitor#on_module_def ?init ?env elem

let validate_class ?init ?env elem = validate visitor#on_class_ ?init ?env elem

let validate_typedef ?init ?env elem =
  validate visitor#on_typedef ?init ?env elem

let validate_fun_def ?init ?env elem =
  validate visitor#on_fun_def ?init ?env elem

let validate_gconst ?init ?env elem = validate visitor#on_gconst ?init ?env elem
