(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Asserter
open Hh_json
open Hh_json.Access
open Hh_json_helpers
open Symbol_add_fact
open Symbol_builder_types
open Symbol_json_util
open OUnit2

let test_add_fact _test_ctxt =
  let progress = init_progress in
  let json_key =
    JSON_Object
      [
        ( "name",
          JSON_Object
            [
              ( "key",
                JSON_Object
                  [("name", JSON_Object [("key", JSON_String "TestName")])] );
            ] );
      ]
  in
  let (res_id, progress) = add_fact ClassDeclaration json_key progress in
  Int_asserter.assert_equals
    1
    (List.length progress.resultJson.classDeclaration)
    "One class decl fact added to JSON";
  let fact_json = List.nth progress.resultJson.classDeclaration 0 in
  let fact_id = Jget.int_d (Some fact_json) "id" (-1) in
  Int_asserter.assert_equals res_id fact_id "Id returned is JSON id of new fact";
  let (res_id2, progress) = add_fact ClassDeclaration json_key progress in
  Int_asserter.assert_equals
    res_id
    res_id2
    "Adding identical facts results in same ids";
  Int_asserter.assert_equals
    1
    (List.length progress.resultJson.classDeclaration)
    "Only one class decl fact in JSON after identical addition";
  let (res_id3, progress) = add_fact FunctionDeclaration json_key progress in
  assert_bool
    "Identical keys for different predicates are separate facts"
    (res_id != res_id3);
  Int_asserter.assert_equals
    1
    (List.length progress.resultJson.functionDeclaration)
    "One function decl fact added to JSON"

let test_add_decl_fact _test_ctxt =
  let progress = init_progress in
  let gconst_name = "TestGConst" in
  let (id, prog) = add_gconst_decl_fact gconst_name progress in
  Int_asserter.assert_equals
    1
    (List.length prog.resultJson.globalConstDeclaration)
    "One gconst fact added";
  let fact_json = List.nth prog.resultJson.globalConstDeclaration 0 in
  let fact_id = Jget.int_d (Some fact_json) "id" (-1) in
  let decl_name =
    return fact_json
    >>= get_obj "key"
    >>= get_obj "name"
    >>= get_obj "key"
    >>= get_obj "name"
    >>= get_string "key"
  in
  Int_asserter.assert_equals id fact_id "Id returned is JSON id of new fact";
  match decl_name with
  | Ok (name, _) ->
    String_asserter.assert_equals gconst_name name "Nested fact contains name"
  | _ -> assert_failure "Could not extract decl name"

let () =
  "write_symbol_info_test"
  >::: [
         "test_add_fact" >:: test_add_fact;
         "test_add_decl_fact" >:: test_add_decl_fact;
       ]
  |> run_test_tt_main
