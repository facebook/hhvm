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

  val consistent_ctor_level : t -> int

  val set_consistent_ctor_level : t -> consistent_ctor_level:int -> t
end = struct
  type t = { consistent_ctor_level: int }

  let empty = { consistent_ctor_level = 0 }

  let consistent_ctor_level { consistent_ctor_level } = consistent_ctor_level

  let set_consistent_ctor_level _ ~consistent_ctor_level =
    { consistent_ctor_level }
end

let on_class_
    (env, (Aast.{ c_methods; c_user_attributes; c_kind; _ } as c), err_acc) =
  let err =
    let attr_pos_opt =
      Naming_attributes.mem_pos
        SN.UserAttributes.uaConsistentConstruct
        c_user_attributes
    in
    let ctor_opt =
      List.find c_methods ~f:(fun Aast.{ m_name = (_, nm); _ } ->
          if String.equal nm "__construct" then
            true
          else
            false)
    in
    match (attr_pos_opt, ctor_opt) with
    | (Some pos, None)
      when Ast_defs.is_c_trait c_kind || Env.consistent_ctor_level env > 1 ->
      if Option.is_none ctor_opt then
        Err.Free_monoid.plus err_acc
        @@ Err.naming
        @@ Naming_error.Explicit_consistent_constructor
             { classish_kind = c_kind; pos }
      else
        err_acc
    | _ -> err_acc
  in
  Naming_phase_pass.Cont.next (env, c, err)

let pass =
  Naming_phase_pass.(top_down { identity with on_class_ = Some on_class_ })

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
