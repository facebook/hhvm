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
  type t = int

  let empty = 0

  let create level = level
end

let visitor =
  object (self)
    inherit [_] Aast_defs.reduce as super

    inherit Err.monoid

    method! on_class_
        env (Aast.{ c_methods; c_user_attributes; c_kind; _ } as c) =
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
        | (Some pos, None) when Ast_defs.is_c_trait c_kind || env > 1 ->
          if Option.is_none ctor_opt then
            Err.naming
            @@ Naming_error.Explicit_consistent_constructor
                 { classish_kind = c_kind; pos }
          else
            self#zero
        | _ -> self#zero
      in
      let super_err = super#on_class_ env c in
      self#plus err super_err
  end

let validate f ?init ?(env = Env.empty) elem =
  Err.from_monoid ?init @@ f env elem

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
