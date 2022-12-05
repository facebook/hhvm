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
    inherit [_] Aast_defs.reduce as super

    inherit Err.monoid

    method private validate_fun_params params =
      snd
      @@ List.fold_left
           params
           ~init:(SSet.empty, self#zero)
           ~f:(fun (seen, err) Aast.{ param_name; param_pos; _ } ->
             if String.equal SN.SpecialIdents.placeholder param_name then
               (seen, err)
             else if SSet.mem param_name seen then
               ( seen,
                 self#plus err
                 @@ Err.naming
                 @@ Naming_error.Already_bound
                      { pos = param_pos; name = param_name } )
             else
               (SSet.add param_name seen, err))

    method! on_method_ env m =
      let err = self#validate_fun_params m.Aast.m_params in
      let super_err = super#on_method_ env m in
      self#plus err super_err

    method! on_fun_ env f =
      let err = self#validate_fun_params f.Aast.f_params in
      let super_err = super#on_fun_ env f in
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
