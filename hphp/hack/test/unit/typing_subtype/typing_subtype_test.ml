(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Typing_defs
module Env = Typing_env
module ITySet = Internal_type_set

let do_test_sub_type_expect_ok ?on_error env sub_ty super_ty =
  let on_error =
    Option.value on_error ~default:(fun _ _ -> failwith "on error was called")
  in
  Typing_subtype.sub_type_i env sub_ty super_ty on_error

let r_none = Typing_reason.Rnone

let no_r t = (r_none, t)

let default_env () =
  let env =
    Typing_env.empty GlobalOptions.default Relative_path.default None
  in
  let env = Env.set_log_level env "sub" 2 in
  let env = Env.set_log_level env "env" 2 in
  let env = Env.set_log_level env "inter" 2 in
  env

(* test (nonnull & _ & #1) <: has_member(m, #2)

   previously this looped as follows:
   loop:
     subtype           (nonnull & _ & #1) <: has_member(m, #2)
     simplify          (nonnull & _ & #1) <: has_member(m, #2)
     simplifies to     (#1 & _) <: ?#3 with upper bound has_member(m, #2)
     simplifies to     (#1 & _ & nonnull) <: #3
     into prop_to_env  (#1 & _ & nonnull) <: #3
     calls simplify on (#1 & _ & nonnull) <: has_member(m, #2)
     back to start *)
let test_intersect_with_nonnull_sub_constraint () =
  let env = default_env () in
  (* construct sub type *)
  let (env, ty1) = Env.fresh_type_reason env r_none in
  let sub_ty =
    no_r
      Typing_defs.(
        Tintersection [no_r Tnonnull; no_r @@ Tany TanySentinel.value; ty1])
  in
  let sub_ty = Typing_defs.LoclType sub_ty in
  (* construct super type *)
  let m = (Pos.none, "m") in
  let (env, mem_ty) = Env.fresh_type_reason env r_none in
  let super_ty = Typing_make_type.has_member r_none m mem_ty Aast.CIself in
  (* do it! *)
  let env = do_test_sub_type_expect_ok env sub_ty super_ty in
  (* check it *)
  let ty1_id = 1 in
  match ITySet.elements @@ Env.get_tyvar_upper_bounds env ty1_id with
  | [LoclType (_, Toption (_, Tvar ty3_id))] ->
    begin
      match ITySet.elements @@ Env.get_tyvar_upper_bounds env ty3_id with
      | [ConstraintType (_, Thas_member _)] -> true
      | _ -> failwith "failed to match upper bound of #3"
    end
  | _ -> failwith "failed to match upper bound of #1"

let tests =
  [
    ( "test_intersect_with_nonnull_sub_constraint",
      test_intersect_with_nonnull_sub_constraint );
  ]

let () = Unit_test.main tests
