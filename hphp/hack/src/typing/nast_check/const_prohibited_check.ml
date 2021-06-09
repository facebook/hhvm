(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
open Nast_check_env
module SN = Naming_special_names

let has_const attrs = Naming_attributes.mem SN.UserAttributes.uaConst attrs

let error_if_const pos attrs =
  if has_const attrs then
    Errors.experimental_feature pos "The __Const attribute is not supported."

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_class_ env c =
      (* Const handling:
       * disallow __Const attribute unless typechecker option is enabled
       *)
      let pos = fst c.c_name in
      if not (TypecheckerOptions.const_attribute (get_tcopt env)) then (
        error_if_const pos c.c_user_attributes;
        List.iter c.c_vars ~f:(fun cv ->
            error_if_const pos cv.cv_user_attributes)
      )
  end
