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
    inherit Tast_visitor.handler_base

    method! at_class_ env c =
      let tcopt = Tast_env.get_tcopt env in
      let enabled = tcopt.GlobalOptions.tco_enable_enum_classes in
      let (pos, c_name) = c.Aast.c_name in
      if (not enabled) && Tast_env.is_enum_class env c_name then
        Errors.enum_classes_reserved_syntax pos
  end
