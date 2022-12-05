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

module Env : sig
  type t

  val empty : t
end = struct
  type t = unit

  let empty = ()
end

let on_hint_ (env, hint_, err_acc) =
  let err =
    match hint_ with
    | Aast.Happly ((pos, ty_name), _)
      when String.(
             equal ty_name SN.Classes.cHH_BuiltinEnum
             || equal ty_name SN.Classes.cHH_BuiltinEnumClass
             || equal ty_name SN.Classes.cHH_BuiltinAbstractEnumClass) ->
      Err.Free_monoid.plus err_acc
      @@ Err.naming
      @@ Naming_error.Using_internal_class
           { pos; class_name = Utils.strip_ns ty_name }
    | _ -> err_acc
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
