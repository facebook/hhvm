(*
 * Copyright (c) 2021, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_class_ env c =
      let tcopt = Nast_check_env.get_tcopt env in
      let enabled = tcopt.GlobalOptions.po_enable_enum_supertyping in
      let is_enum_class = Ast_defs.is_c_enum_class c.Aast.c_kind in
      let uses_enum_supertyping =
        match c.Aast.c_enum with
        | Some enum_ ->
          (not (List.is_empty enum_.Aast.e_includes)) && not is_enum_class
        | None -> false
      in
      let pos = fst c.Aast.c_name in
      if (not enabled) && uses_enum_supertyping then
        Errors.add_nast_check_error
        @@ Nast_check_error.Enum_supertyping_reserved_syntax { pos }
  end
