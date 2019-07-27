(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Nast
open Nast_check_env

module SN = Naming_special_names

let has_const attrs =
  Attributes.mem SN.UserAttributes.uaConst attrs

let error_if_const pos attrs =
  if has_const attrs then
    Errors.experimental_feature pos "The __Const attribute is not supported."

let handler = object
  inherit Nast_visitor.handler_base

  method! at_class_ env c =
    (* Const handling:
     * prevent for abstract final classes, traits, and interfaces
     *)
    let pos = (fst c.c_name) in
    if not (TypecheckerOptions.const_attribute env.tcopt) then begin
      error_if_const pos c.c_user_attributes;
      List.iter c.c_vars (fun cv -> error_if_const pos cv.cv_user_attributes)
    end
    else if has_const c.c_user_attributes then
      match c.c_kind, c.c_final with
      | Ast.Cabstract, true
      | Ast.Cinterface, _
      | Ast.Ctrait, _
      | Ast.Cenum, _
      | Ast.Crecord, _ ->
        Errors.const_attribute_prohibited
          pos (Typing_print.class_kind c.c_kind c.c_final);
      | Ast.Cabstract, false
      | Ast.Cnormal, _ -> ();
end
