(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Typing_defs
open Typing_env_types
module Reason = Typing_reason
module Env = Typing_env
module TUtils = Typing_utils
module MakeType = Typing_make_type

let is_object env ty = Typing_solver.is_sub_type env ty (Reason.Rnone, Tobject)

let sub_string (p : Pos.Map.key) (env : env) (ty : locl_ty) : env =
  (* Under constraint-based inference, we implement sub_string as a subtype test.
   * All the cases in the legacy implementation just fall out from subtyping rules.
   * We test against ?(arraykey | bool | float | resource | object | dynamic |
   * FormatString<T> | HH\FormatString<T>).
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
      MakeType.class_type r SN.Classes.cFormatString [formatter_tyvar];
      MakeType.class_type r SN.Classes.cHHFormatString [formatter_tyvar];
    ]
  in
  let stringish =
    (Reason.Rwitness p, Tclass ((p, SN.Classes.cStringish), Nonexact, []))
  in
  let stringlike =
    (Reason.Rwitness p, Toption (Reason.Rwitness p, Tunion tyl))
  in
  Errors.try_
    (fun () ->
      Typing_subtype.sub_type env ty stringlike Errors.expected_stringlike)
    (fun _ ->
      if Typing_solver.is_sub_type env ty stringish then
        Errors.object_string_deprecated p
      else if is_object env ty then
        Errors.object_string p (Reason.to_pos (fst ty))
      else
        Errors.invalid_sub_string p (Typing_print.error env ty);
      env)
