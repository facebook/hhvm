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

(* TODO: delete this check *)
let error_if_abstract_type_constant_with_default env typeconst =
  if
    not
      (TypecheckerOptions.experimental_feature_enabled
         (get_tcopt env)
         TypecheckerOptions.experimental_abstract_type_const_with_default)
  then
    match typeconst.c_tconst_kind with
    | Aast.TCAbstract { c_atc_default = Some _; _ } ->
      let (pos, _) = typeconst.c_tconst_name in
      Errors.experimental_feature pos "abstract type constant with default"
    | _ -> ()

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_class_ env c =
      let check_typeconst typeconst =
        error_if_abstract_type_constant_with_default env typeconst;
        ()
      in
      List.iter c.c_typeconsts ~f:check_typeconst
  end
