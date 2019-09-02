(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Aast
open Nast_check_env
module SN = Naming_special_names

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_class_ env cv =
      let check_vars cv =
        let pos = fst cv.cv_id in
        if
          cv.cv_is_static
          && (not (TypecheckerOptions.const_static_props env.tcopt))
          && Attributes.mem SN.UserAttributes.uaConst cv.cv_user_attributes
        then
          Errors.experimental_feature pos "Const properties cannot be static."
        else
          ()
      in
      List.iter cv.c_vars check_vars
  end
