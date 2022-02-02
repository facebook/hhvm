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

let sub_string_err (p : Pos.t) (env : env) (ty : locl_ty) :
    env * (locl_ty * locl_ty) option =
  (* Under constraint-based inference, we implement sub_string as a subtype test.
   * All the cases in the legacy implementation just fall out from subtyping rules.
   * We test against ?(arraykey | bool | float | resource | dynamic |
   * HH\FormatString<T>).
   *)
  let r = Reason.Rwitness p in
  let (env, formatter_tyvar) = Env.fresh_type_invariant env p in
  let tyl =
    [
      MakeType.arraykey r;
      MakeType.bool r;
      MakeType.float r;
      MakeType.resource r;
      MakeType.dynamic r;
      MakeType.hh_formatstring r formatter_tyvar;
    ]
  in
  let stringish = MakeType.class_type r SN.Classes.cStringish [] in
  let stringlike = MakeType.nullable_locl r (MakeType.union r tyl) in
  Result.fold
    ~ok:(fun env -> (env, None))
    ~error:(fun env -> (env, Some (ty, stringlike)))
  @@ Typing_subtype.sub_type_or_fail_res env ty stringlike
  @@ Some
       Typing_error.(
         primary
         @@
         if Typing_solver.is_sub_type env ty stringish then
           Primary.Object_string_deprecated p
         else
           Primary.Invalid_substring
             { pos = p; ty_name = lazy (Typing_print.error env ty) })

let sub_string (p : Pos.t) (env : env) (ty : locl_ty) : env =
  fst @@ sub_string_err p env ty
