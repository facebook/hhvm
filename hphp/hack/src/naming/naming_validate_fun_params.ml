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

let allow_ignore_readonly Naming_phase_env.{ allow_ignore_readonly; _ } =
  allow_ignore_readonly

let validate_fun_params ~ctx params =
  snd
  @@ List.fold_left
       params
       ~init:(SSet.empty, [])
       ~f:(fun
            (seen, errs)
            Aast.{ param_name; param_pos; param_user_attributes; _ }
          ->
         let errs =
           match
             ( Naming_attributes.mem_pos
                 SN.UserAttributes.uaIgnoreReadonlyError
                 param_user_attributes,
               allow_ignore_readonly ctx )
           with
           | (Some attr_pos, false) ->
             let err =
               Err.naming
               @@ Naming_error.Attribute_outside_allowed_files attr_pos
             in
             err :: errs
           | _ -> errs
         in
         if String.equal SN.SpecialIdents.placeholder param_name then
           (seen, errs)
         else if SSet.mem param_name seen then
           let err =
             Err.naming
             @@ Naming_error.Already_bound
                  { pos = param_pos; name = param_name }
           in
           (seen, err :: errs)
         else if String.equal param_name SN.SpecialIdents.this then
           let err =
             Err.naming @@ Naming_error.This_as_lexical_variable param_pos
           in
           (seen, err :: errs)
         else
           (SSet.add param_name seen, errs))

let on_method_ on_error m ~ctx =
  List.iter ~f:on_error @@ (validate_fun_params ~ctx) m.Aast.m_params;
  (ctx, Ok m)

let on_fun_ on_error f ~ctx =
  List.iter ~f:on_error @@ (validate_fun_params ~ctx) f.Aast.f_params;
  (ctx, Ok f)

let pass on_error =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.top_down
    Aast.Pass.
      {
        id with
        on_ty_method_ = Some (fun elem ~ctx -> on_method_ on_error elem ~ctx);
        on_ty_fun_ = Some (fun elem ~ctx -> on_fun_ on_error elem ~ctx);
      }
