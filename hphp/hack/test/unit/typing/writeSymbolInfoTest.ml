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
open Symbol_build_json
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
  let fact_id = Jget.int_d fact_json "id" ~default:(-1) in
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
  let fact_json = List.nth_exn prog.resultJson.globalConstDeclaration 0 in
  let fact_id = Jget.int_d (Some fact_json) "id" ~default:(-1) in
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

let test_build_xrefs _test_ctxt =
  let xrefs =
    (SMap.empty : (Hh_json.json * Relative_path.t Pos.pos list) IMap.t SMap.t)
  in
  Relative_path.set_path_prefix Relative_path.Root (Path.make "www");
  let file = Relative_path.from_root ~suffix:"test.php" in
  let decl_name = "TestDecl" in
  let target_json = JSON_Object [("declaration", JSON_String decl_name)] in
  let target_id = 1 in
  let ref_pos =
    Pos.set_file
      file
      (Pos.make_from_lnum_bol_cnum
         ~pos_file:file
         ~pos_start:(2, 5, 10)
         ~pos_end:(2, 5, 15))
  in
  let dup_ref_pos =
    Pos.set_file
      file
      (Pos.make_from_lnum_bol_cnum
         ~pos_file:file
         ~pos_start:(2, 5, 10)
         ~pos_end:(2, 5, 15))
  in
  let next_ref_pos =
    Pos.set_file
      file
      (Pos.make_from_lnum_bol_cnum
         ~pos_file:file
         ~pos_start:(3, 25, 40)
         ~pos_end:(3, 25, 45))
  in
  let xrefs = add_xref target_json target_id next_ref_pos xrefs in
  let xrefs = add_xref target_json target_id ref_pos xrefs in
  let xrefs = add_xref target_json target_id dup_ref_pos xrefs in
  let file_map : (Hh_json.json * Pos.t list) IMap.t =
    SMap.find (Relative_path.to_absolute file) xrefs
  in
  let result = List.nth_exn (get_array_exn (build_xrefs_json file_map)) 0 in
  let target_decl =
    return result >>= get_obj "target" >>= get_string "declaration"
  in
  (match target_decl with
  | Ok (name, _) ->
    String_asserter.assert_equals decl_name name "Target decl JSON set"
  | _ -> assert_failure "Could not extract decl JSON");
  let ranges_arr = return result >>= get_array "ranges" in
  match ranges_arr with
  | Ok (ranges, _) ->
    Int_asserter.assert_equals
      2
      (List.length ranges)
      "Duplicate references removed";
    let offset = return (List.nth_exn ranges 1) >>= get_number_int "offset" in
    (match offset with
    | Ok (offset2, _) ->
      Int_asserter.assert_equals
        30
        offset2
        "Byte offset between references calculated correctly"
    | _ -> assert_failure "Could not extract offset")
  | _ -> assert_failure "Could not extract ranges"

let () =
  "write_symbol_info_test"
  >::: [
         "test_add_fact" >:: test_add_fact;
         "test_add_decl_fact" >:: test_add_decl_fact;
         "test_build_xrefs" >:: test_build_xrefs;
       ]
  |> run_test_tt_main
