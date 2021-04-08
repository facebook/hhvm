(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs
open Typing_env_types
module Reason = Typing_reason
module Env = Typing_env
module MakeType = Typing_make_type
module SN = Naming_special_names

let is_object env ty =
  Typing_solver.is_sub_type env ty (MakeType.ty_object (get_reason ty))

let sub_string (p : Pos.t) (env : env) (ty : locl_ty) : env =
  (* Under constraint-based inference, we implement sub_string as a subtype test.
   * All the cases in the legacy implementation just fall out from subtyping rules.
   * We test against ?(arraykey | bool | float | resource | object | dynamic |
   * HH\FormatString<T>).
   *)
  let r = Reason.Rwitness p in
  let (env, formatter_tyvar) = Env.fresh_invariant_type_var env p in
  let tyl =
    [
      MakeType.arraykey r;
      MakeType.bool r;
      MakeType.float r;
      MakeType.resource r;
      MakeType.dynamic r;
      MakeType.new_type r SN.Classes.cHHFormatString [formatter_tyvar];
    ]
  in
  let stringish = MakeType.class_type r SN.Classes.cStringish [] in
  let stringlike = MakeType.nullable_locl r (MakeType.union r tyl) in
  Typing_subtype.sub_type_or_fail env ty stringlike (fun () ->
      if Typing_solver.is_sub_type env ty stringish then
        Errors.object_string_deprecated p
      else if is_object env ty then
        Errors.object_string p (get_pos ty)
      else
        Errors.invalid_sub_string p (Typing_print.error env ty))
