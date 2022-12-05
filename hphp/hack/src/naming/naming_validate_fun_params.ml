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
