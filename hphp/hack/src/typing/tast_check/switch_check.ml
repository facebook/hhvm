(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core
open Tast
open Typing_defs

module Env = Tast_env

let ensure_valid_switch_case_value_types env scrutinee_ty casel errorf =
  let is_subtype ty_sub ty_super = snd (Env.subtype env ty_sub ty_super) in
  let ty_num = (Reason.Rnone, Tprim Nast.Tnum) in
  let ty_arraykey = (Reason.Rnone, Tprim Nast.Tarraykey) in
  let compatible_types ty1 ty2 =
    (is_subtype ty1 ty_num && is_subtype ty2 ty_num) ||
    (is_subtype ty1 ty_arraykey && is_subtype ty2 ty_arraykey) ||
    (is_subtype ty1 ty2 && is_subtype ty2 ty1) in
  let ensure_valid_switch_case_value_type = function
    | Default _ -> ()
    | Case (((case_value_p, case_value_ty), _), _) ->
      if not (compatible_types case_value_ty scrutinee_ty) then
        errorf (Env.get_tcopt env) case_value_p
          (Env.print_ty env case_value_ty) (Env.print_ty env scrutinee_ty) in
  List.iter casel ensure_valid_switch_case_value_type

let handler errorf = object
  inherit Tast_visitor.handler_base

  method! at_stmt env x =
    match x with
    | Switch (((_, scrutinee_ty), _), casel) ->
      ensure_valid_switch_case_value_types env scrutinee_ty casel errorf
    | _ -> ()
end
