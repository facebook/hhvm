(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open OUnit2
open Typing_defs
open Typing_env_types
module Env = Typing_env
module Inf = Typing_inference_env
module Log = Typing_log
module MakeType = Typing_make_type
module Reason = Typing_reason
module U = Typing_union

module Helpers : sig
  val dummy_env : env

  val tint : locl_ty

  val fresh_tyvar : env -> env * locl_ty * Tvid.t

  val union : env -> locl_ty -> locl_ty -> env * locl_ty * Tvid.t

  val assert_tyvars_contain : env -> Tvid.t list Tvid.Map.t -> unit

  val assert_ty_equal : locl_ty -> locl_ty -> unit

  val assert_are_alias_for_another_var : env -> Tvid.t list -> unit

  val show_env : env -> unit [@@warning "-unused-value-declaration"]
end = struct
  let () = Typing_subtype.set_fun_refs ()

  let dummy_env =
    let ctx =
      Provider_context.empty_for_test
        ~popt:ParserOptions.default
        ~tcopt:TypecheckerOptions.default
        ~deps_mode:(Typing_deps_mode.InMemoryMode None)
    in
    let env = Typing_env_types.empty ctx Relative_path.default ~droot:None in
    let env = Env.set_log_level env "show" 2 in
    env

  let tint = MakeType.int Reason.none

  let fresh_tyvar env =
    let (env, tv) = Env.fresh_type_reason env Pos.none Reason.none in
    match get_node tv with
    | Tvar v -> (env, tv, v)
    | _ -> assert_failure "fresh_type_reason should return a type var"

  let union env ty1 ty2 =
    let (env, tunion) = U.union env ty1 ty2 in
    match get_node tunion with
    | Tvar v -> (env, tunion, v)
    | _ -> assert_failure "A union should always be wrapped in a type variable."

  let assert_tyvar_occurs_in_tyvar ?(negate = false) env tv1 ~in_:tv2 =
    let error_message =
      Printf.sprintf
        "Type var #%s should %scontain type var #%s"
        (Tvid.show tv2)
        (if negate then
          "not "
        else
          "")
        (Tvid.show tv1)
    in
    assert_bool
      error_message
      (not
         (Bool.equal
            negate
            (Inf.tyvar_occurs_in_tyvar env.inference_env tv1 ~in_:tv2)))

  let assert_tyvar_doesnt_occur_in_tyvar =
    assert_tyvar_occurs_in_tyvar ~negate:true

  let assert_tyvar_contains env ?(contains = []) ?(doesnt_contain = []) tv =
    List.iter contains ~f:(assert_tyvar_occurs_in_tyvar env ~in_:tv);
    List.iter doesnt_contain ~f:(assert_tyvar_doesnt_occur_in_tyvar env ~in_:tv)

  let assert_tyvars_contain env contain_map =
    let alltv = Tvid.Map.keys contain_map |> Tvid.Set.of_list in
    Tvid.Map.iter
      (fun v contains ->
        let doesnt_contain =
          Tvid.Set.diff alltv (Tvid.Set.of_list contains) |> Tvid.Set.elements
        in
        assert_tyvar_contains env v ~contains ~doesnt_contain)
      contain_map

  let assert_ty_equal ty1 ty2 = assert_equal (get_node ty1) (get_node ty2)

  let assert_are_alias_for_another_var env vars =
    List.iter vars ~f:(fun v ->
        assert_bool
          (Printf.sprintf
             "Variable #%s should be an alias for another variable."
             (Tvid.show v))
          (Inf.is_alias_for_another_var env.inference_env v))

  let show_env env = Log.hh_show_env Pos.none env
end

open Helpers

let int_union _test_ctx =
  let env = dummy_env in
  let (env, tv1, n1) = fresh_tyvar env in
  let (env, tv2, n2) = fresh_tyvar env in
  let (env, tunion, nu) = union env tv1 tv2 in
  assert_tyvars_contain env
  @@ Tvid.Map.of_list [(nu, [n1; n2]); (n1, []); (n2, [])];
  let env = Env.add env n1 tint in
  let env = Env.add env n2 tint in
  let (env, etunion) = Env.expand_type env tunion in
  assert_ty_equal etunion tint;
  assert_tyvars_contain env @@ Tvid.Map.of_list [(nu, []); (n1, []); (n2, [])]

let alias_chain _ =
  let env = dummy_env in
  let (env, tv1, v1) = fresh_tyvar env in
  let (env, tv2, v2) = fresh_tyvar env in
  let env = Env.add env v2 tv1 in
  assert_tyvars_contain env @@ Tvid.Map.of_list [(v1, []); (v2, [v1])];
  let (env, tv3, v3) = fresh_tyvar env in
  let env = Env.add env v3 tv2 in
  assert_tyvars_contain env
  @@ Tvid.Map.of_list [(v1, []); (v2, [v1]); (v3, [v2])];
  let (env, tv4, v4) = fresh_tyvar env in
  let env = Env.add env v4 tv3 in
  let (env, _ty) = Env.expand_type env tv4 in
  assert_tyvars_contain env
  @@ Tvid.Map.of_list [(v1, []); (v2, [v1]); (v3, [v1]); (v4, [v1])];
  let env = Env.add env v1 tint in
  assert_tyvars_contain env
  @@ Tvid.Map.of_list [(v1, []); (v2, []); (v3, []); (v4, [])];
  assert_are_alias_for_another_var env [v2; v3; v4]

let union_of_union _ =
  let env = dummy_env in
  let (env, tv1l, v1l) = fresh_tyvar env in
  let (env, tv1r, v1r) = fresh_tyvar env in
  let (env, tvu1, vu1) = union env tv1l tv1r in
  let (env, tv2l, v2l) = fresh_tyvar env in
  let (env, tv2r, v2r) = fresh_tyvar env in
  let (env, tvu2, vu2) = union env tv2l tv2r in
  assert_tyvars_contain env
  @@ Tvid.Map.of_list
       [
         (vu2, [v2l; v2r]);
         (vu1, [v1l; v1r]);
         (v1l, []);
         (v1r, []);
         (v2l, []);
         (v2r, []);
       ];
  let env = Env.add env v2l tvu1 in
  assert_tyvars_contain env
  @@ Tvid.Map.of_list
       [
         (vu2, [v2l; v2r]);
         (vu1, [v1l; v1r]);
         (v1l, []);
         (v1r, []);
         (v2l, [vu1]);
         (v2r, []);
       ];
  let env = Env.add env v1l tint in
  assert_tyvars_contain env
  @@ Tvid.Map.of_list
       [
         (vu2, [v2l; v2r]);
         (vu1, [v1r]);
         (v1l, []);
         (v1r, []);
         (v2l, [vu1]);
         (v2r, []);
       ];
  let env = Env.add env v2r tint in
  assert_tyvars_contain env
  @@ Tvid.Map.of_list
       [
         (vu2, [v2l]);
         (vu1, [v1r]);
         (v1l, []);
         (v1r, []);
         (v2l, [vu1]);
         (v2r, []);
       ];
  let env = Env.add env v1r tint in
  assert_tyvars_contain env
  @@ Tvid.Map.of_list
       [(vu2, []); (vu1, []); (v1l, []); (v1r, []); (v2l, []); (v2r, [])];
  let (env, ty1) = Env.expand_type env tvu1 in
  assert_ty_equal ty1 tint;
  let (env, ty2) = Env.expand_type env tvu2 in
  assert_ty_equal ty2 tint;
  assert_are_alias_for_another_var env [v2l]

let union_of_union_w_chain1 _ =
  let env = dummy_env in
  let (env, tv1l, v1l) = fresh_tyvar env in
  let (env, tv1r, v1r) = fresh_tyvar env in
  let (env, tvu1, vu1) = union env tv1l tv1r in
  let (env, tv2l, v2l) = fresh_tyvar env in
  let (env, tv2r, v2r) = fresh_tyvar env in
  let (env, tvu2, vu2) = union env tv2l tv2r in
  assert_tyvars_contain env
  @@ Tvid.Map.of_list
       [
         (vu2, [v2l; v2r]);
         (vu1, [v1l; v1r]);
         (v1l, []);
         (v1r, []);
         (v2l, []);
         (v2r, []);
       ];
  let (env, tvch1, vch1) = fresh_tyvar env in
  let env = Env.add env v2l tvch1 in
  assert_tyvars_contain env
  @@ Tvid.Map.of_list
       [
         (vu2, [v2l; v2r]);
         (vu1, [v1l; v1r]);
         (v1l, []);
         (v1r, []);
         (v2l, [vch1]);
         (v2r, []);
         (vch1, []);
       ];
  let env = Env.add env vch1 tvu1 in
  assert_tyvars_contain env
  @@ Tvid.Map.of_list
       [
         (vu2, [v2l; v2r]);
         (vu1, [v1l; v1r]);
         (v1l, []);
         (v1r, []);
         (v2l, [vch1]);
         (v2r, []);
         (vch1, [vu1]);
       ];
  let env = Env.add env v1l tint in
  assert_tyvars_contain env
  @@ Tvid.Map.of_list
       [
         (vu2, [v2l; v2r]);
         (vu1, [v1r]);
         (v1l, []);
         (v1r, []);
         (v2l, [vch1]);
         (v2r, []);
         (vch1, [vu1]);
       ];
  let env = Env.add env v2r tint in
  assert_tyvars_contain env
  @@ Tvid.Map.of_list
       [
         (vu2, [v2l]);
         (vu1, [v1r]);
         (v1l, []);
         (v1r, []);
         (v2l, [vch1]);
         (v2r, []);
         (vch1, [vu1]);
       ];
  let env = Env.add env v1r tint in
  assert_tyvars_contain env
  @@ Tvid.Map.of_list
       [
         (vu2, []);
         (vu1, []);
         (v1l, []);
         (v1r, []);
         (v2l, []);
         (v2r, []);
         (vch1, []);
       ];
  let (env, ty1) = Env.expand_type env tvu1 in
  assert_ty_equal ty1 tint;
  let (env, ty2) = Env.expand_type env tvu2 in
  assert_ty_equal ty2 tint;
  assert_are_alias_for_another_var env [v2l; vch1]

let union_of_union_w_chain2 _ =
  let env = dummy_env in
  let (env, tv1l, v1l) = fresh_tyvar env in
  let (env, tv1r, v1r) = fresh_tyvar env in
  let (env, tvu1, vu1) = union env tv1l tv1r in
  let (env, tv2l, v2l) = fresh_tyvar env in
  let (env, tv2r, v2r) = fresh_tyvar env in
  let (env, tvu2, vu2) = union env tv2l tv2r in
  assert_tyvars_contain env
  @@ Tvid.Map.of_list
       [
         (vu2, [v2l; v2r]);
         (vu1, [v1l; v1r]);
         (v1l, []);
         (v1r, []);
         (v2l, []);
         (v2r, []);
       ];
  let (env, tvch1, vch1) = fresh_tyvar env in
  let env = Env.add env v2l tvch1 in
  assert_tyvars_contain env
  @@ Tvid.Map.of_list
       [
         (vu2, [v2l; v2r]);
         (vu1, [v1l; v1r]);
         (v1l, []);
         (v1r, []);
         (v2l, [vch1]);
         (v2r, []);
         (vch1, []);
       ];
  let (env, tvch2, vch2) = fresh_tyvar env in
  let env = Env.add env vch1 tvch2 in
  assert_tyvars_contain env
  @@ Tvid.Map.of_list
       [
         (vu2, [v2l; v2r]);
         (vu1, [v1l; v1r]);
         (v1l, []);
         (v1r, []);
         (v2l, [vch1]);
         (v2r, []);
         (vch1, [vch2]);
         (vch2, []);
       ];
  let env = Env.add env vch2 tvu1 in
  assert_tyvars_contain env
  @@ Tvid.Map.of_list
       [
         (vu2, [v2l; v2r]);
         (vu1, [v1l; v1r]);
         (v1l, []);
         (v1r, []);
         (v2l, [vch1]);
         (v2r, []);
         (vch1, [vch2]);
         (vch2, [vu1]);
       ];
  let env = Env.add env v1l tint in
  assert_tyvars_contain env
  @@ Tvid.Map.of_list
       [
         (vu2, [v2l; v2r]);
         (vu1, [v1r]);
         (v1l, []);
         (v1r, []);
         (v2l, [vch1]);
         (v2r, []);
         (vch1, [vch2]);
         (vch2, [vu1]);
       ];
  let env = Env.add env v2r tint in
  assert_tyvars_contain env
  @@ Tvid.Map.of_list
       [
         (vu2, [v2l]);
         (vu1, [v1r]);
         (v1l, []);
         (v1r, []);
         (v2l, [vch1]);
         (v2r, []);
         (vch1, [vch2]);
         (vch2, [vu1]);
       ];
  let env = Env.add env v1r tint in
  assert_tyvars_contain env
  @@ Tvid.Map.of_list
       [
         (vu2, []);
         (vu1, []);
         (v1l, []);
         (v1r, []);
         (v2l, []);
         (v2r, []);
         (vch1, []);
         (vch2, []);
       ];
  let (env, ty1) = Env.expand_type env tvu1 in
  assert_ty_equal ty1 tint;
  let (env, ty2) = Env.expand_type env tvu2 in
  assert_ty_equal ty2 tint;
  assert_are_alias_for_another_var env [v2l; vch1; vch2]

let diamond _ =
  let env = dummy_env in
  let (env, tvl, vl) = fresh_tyvar env in
  let (env, tvr, vr) = fresh_tyvar env in
  let (env, tvu, vu) = union env tvl tvr in
  assert_tyvars_contain env
  @@ Tvid.Map.of_list [(vl, []); (vr, []); (vu, [vl; vr])];
  let (env, tv, v) = fresh_tyvar env in
  let env = Env.add env vl tv in
  let env = Env.add env vr tv in
  assert_tyvars_contain env
  @@ Tvid.Map.of_list [(v, []); (vl, [v]); (vr, [v]); (vu, [vl; vr])];
  let env = Env.add env v tint in
  assert_tyvars_contain env
  @@ Tvid.Map.of_list [(v, []); (vl, []); (vr, []); (vu, [])];
  let (env, ty) = Env.expand_type env tvu in
  assert_ty_equal ty tint;
  assert_are_alias_for_another_var env [vl; vr]

let () =
  "typeSimplifierTest"
  >::: [
         "int_union" >:: int_union;
         "alias_chain" >:: alias_chain;
         "union_of_union" >:: union_of_union;
         "union_of_union_w_chain1" >:: union_of_union_w_chain1;
         "union_of_union_w_chain2" >:: union_of_union_w_chain2;
         "diamond" >:: diamond;
       ]
  |> run_test_tt_main
