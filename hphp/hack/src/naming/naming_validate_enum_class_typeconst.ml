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

let on_class_ (env, (Aast.{ c_kind; c_typeconsts; c_span; _ } as c), err_acc) =
  let err =
    if (not (List.is_empty c_typeconsts)) && Ast_defs.is_c_enum_class c_kind
    then
      Err.Free_monoid.plus err_acc
      @@ Err.naming
      @@ Naming_error.Type_constant_in_enum_class_outside_allowed_locations
           c_span
    else
      err_acc
  in
  Naming_phase_pass.Cont.next (env, c, err)

let pass =
  Naming_phase_pass.(top_down { identity with on_class_ = Some on_class_ })

let visitor = Naming_phase_pass.mk_visitor [pass]

let validate ?init ?(env = Env.empty) f elem =
  Err.from_monoid ?init @@ snd @@ f env elem

let validate_program ?init ?env prog =
  validate ?init ?env visitor#on_program prog

let validate_module_def ?init ?env elem =
  validate ?init ?env visitor#on_module_def elem

let validate_class ?init ?env elem = validate ?init ?env visitor#on_class_ elem

let validate_typedef ?init ?env elem =
  validate ?init ?env visitor#on_typedef elem

let validate_fun_def ?init ?env elem =
  validate ?init ?env visitor#on_fun_def elem

let validate_gconst ?init ?env elem = validate ?init ?env visitor#on_gconst elem
