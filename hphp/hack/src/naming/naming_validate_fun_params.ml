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

let validate_fun_params params =
  snd
  @@ List.fold_left
       params
       ~init:(SSet.empty, [])
       ~f:(fun (seen, errs) Aast.{ param_name; param_pos; _ } ->
         if String.equal SN.SpecialIdents.placeholder param_name then
           (seen, errs)
         else if SSet.mem param_name seen then
           let err =
             Err.naming
             @@ Naming_error.Already_bound
                  { pos = param_pos; name = param_name }
           in
           (seen, err :: errs)
         else
           (SSet.add param_name seen, errs))

let on_method_ (env, m, err) =
  let err =
    List.fold_right ~init:err ~f:Err.Free_monoid.plus
    @@ validate_fun_params m.Aast.m_params
  in
  Naming_phase_pass.Cont.next (env, m, err)

let on_fun_ (env, f, err) =
  let err =
    List.fold_right ~init:err ~f:Err.Free_monoid.plus
    @@ validate_fun_params f.Aast.f_params
  in
  Naming_phase_pass.Cont.next (env, f, err)

let pass =
  Naming_phase_pass.(
    top_down
      { identity with on_method_ = Some on_method_; on_fun_ = Some on_fun_ })

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
