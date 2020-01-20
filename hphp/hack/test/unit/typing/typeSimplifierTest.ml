(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open OUnit2
open Typing_defs
open Typing_env_types
module Env = Typing_env
module Inf = Typing_inference_env
module MakeType = Typing_make_type
module Reason = Typing_reason
module Solver = Typing_solver
module U = Typing_union

let () = Typing_subtype.set_fun_refs ()

let dummy_env =
  Env.empty TypecheckerOptions.default Relative_path.default ~droot:None

let assert_tyvar_occurs_in_tyvar ?(negate = false) env tv1 ~in_:tv2 =
  let error_message =
    Printf.sprintf
      "Type var %d should %scontain type var %d"
      tv2
      ( if negate then
        "not "
      else
        "" )
      tv1
  in
  assert_bool
    error_message
    (negate <> Inf.tyvar_occurs_in_tyvar env.inference_env tv1 ~in_:tv2)

let assert_tyvar_doesnt_occur_in_tyvar =
  assert_tyvar_occurs_in_tyvar ~negate:true

let assert_tyvar_contains env tv ~contains:tvs =
  List.iter tvs ~f:(assert_tyvar_occurs_in_tyvar env ~in_:tv)

let assert_tyvar_doesnt_contain env tv ~doesnt_contain:tvs =
  List.iter tvs ~f:(assert_tyvar_doesnt_occur_in_tyvar env ~in_:tv)

let test1 _test_ctx =
  let env = dummy_env in
  let (env, tv1) = Env.fresh_type_reason env Reason.none in
  let (env, tv2) = Env.fresh_type_reason env Reason.none in
  let (env, tunion) = U.union env tv1 tv2 in
  match (tv1, tv2, tunion) with
  | ((_, Tvar n1), (_, Tvar n2), (_, Tvar nu)) ->
    assert_tyvar_contains env nu ~contains:[n1; n2];
    let tint = MakeType.int Reason.none in
    let env = Solver.bind env n1 tint in
    let env = Solver.bind env n2 tint in
    let (env, etunion) = Env.expand_type env tunion in
    assert_equal (snd etunion) (snd tint);
    assert_tyvar_doesnt_contain env nu ~doesnt_contain:[n1; n2]
  | _ -> assert_failure "A union should always be wrapped in a type variable."

let () = "typeSimplifierTest" >::: ["test1" >:: test1] |> run_test_tt_main
