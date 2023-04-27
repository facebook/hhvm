(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
module SN = Naming_special_names

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_class_ env c =
      if Option.is_some env.Nast_check_env.module_ then
        match c.c_kind with
        | Ast_defs.Ctrait when not c.c_internal ->
          let trait_pos = fst c.c_name in
          let check visibility pos member =
            if Aast.equal_visibility Aast.Internal (visibility member) then
              Errors.add_nast_check_error
              @@ Nast_check_error.Internal_member_inside_public_trait
                   { member_pos = pos member; trait_pos }
          in
          List.iter
            c.c_methods
            ~f:(check (fun meth -> meth.m_visibility) (fun meth -> meth.m_span));
          List.iter
            c.c_vars
            ~f:(check (fun cv -> cv.cv_visibility) (fun cv -> cv.cv_span))
        | Ast_defs.Ctrait
        | Ast_defs.Cclass _
        | Ast_defs.Cinterface
        | Ast_defs.Cenum
        | Ast_defs.Cenum_class _ ->
          ()
  end
