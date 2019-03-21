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

module SN = Naming_special_names

let handler = object
  inherit Nast_visitor.handler_base

  method! at_class_ _env c =
    (* Const handling:
     * prevent for abstract final classes, traits, and interfaces
     *)
    let pos = (fst c.c_name) in
    if Attributes.mem SN.UserAttributes.uaConst c.c_user_attributes
    then begin
    (* Temporarily ban the __Const attribute entirely *)
    Errors.experimental_feature pos "The __Const attribute is not supported.";
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
end
