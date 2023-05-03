(*
 * Copyright (c) 2021, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Env = Tast_env

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_class_ env c =
      let tcopt = Env.get_tcopt env in
      let enabled = tcopt.GlobalOptions.po_enable_enum_classes in
      let is_enum_class = Aast.is_enum_class c in
      let pos = fst c.Aast.c_name in
      if (not enabled) && is_enum_class then
        Typing_error_utils.add_typing_error
          Typing_error.(enum @@ Primary.Enum.Enum_classes_reserved_syntax pos)
  end
