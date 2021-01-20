(*
 * Copyright (c) 2021, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_class_ env c =
      let tcopt = Nast_check_env.get_tcopt env in
      let enabled = tcopt.GlobalOptions.po_enable_enum_classes in
      let is_enum_class =
        match c.Aast.c_enum with
        | Some enum_ -> enum_.Aast.e_enum_class
        | None -> false
      in
      let pos = fst c.Aast.c_name in
      if (not enabled) && is_enum_class then
        Errors.enum_classes_reserved_syntax pos
  end
