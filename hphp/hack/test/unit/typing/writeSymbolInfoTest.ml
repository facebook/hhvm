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
open OUnit2
module Util = Symbol_json_util
module Build_json = Symbol_build_json
module Predicate = Symbol_predicate
module Add_fact = Symbol_add_fact
module Fact_id = Symbol_fact_id
module Fact_acc = Symbol_predicate.Fact_acc
module XRefs = Symbol_xrefs

let extract_facts_from_obj pred_name = function
  | JSON_Object [("predicate", JSON_String p); ("facts", JSON_Array l)]
    when p = pred_name ->
    Some l
  | _ -> None

let extract_facts_exn pred_name json_objects =
  match List.filter_map ~f:(extract_facts_from_obj pred_name) json_objects with
  | [facts] -> facts
  | _ -> failwith ("There should be exactly one predicate " ^ pred_name)

let test_add_fact _test_ctxt =
  let progress = Fact_acc.init ~ownership:false in
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
  let (res_id, progress) =
    Fact_acc.add_fact Predicate.(Hack ClassDeclaration) json_key progress
  in
  let facts_class_declaration =
    extract_facts_exn "hack.ClassDeclaration.6" (Fact_acc.to_json progress)
  in
  Int_asserter.assert_equals
    1
    (List.length facts_class_declaration)
    "One class decl fact added to JSON";
  let fact_json = List.nth facts_class_declaration 0 in
  let fact_id = Jget.int_d fact_json "id" ~default:(-1) in
  Int_asserter.assert_equals
    (res_id :> int)
    fact_id
    "Id returned is JSON id of new fact";
  let (res_id2, progress) =
    Fact_acc.add_fact Predicate.(Hack ClassDeclaration) json_key progress
  in
  let facts_class_declaration =
    extract_facts_exn "hack.ClassDeclaration.6" (Fact_acc.to_json progress)
  in
  Int_asserter.assert_equals
    (res_id :> int)
    (res_id2 :> int)
    "Adding identical facts results in same ids";
  Int_asserter.assert_equals
    1
    (List.length facts_class_declaration)
    "Only one class decl fact in JSON after identical addition";
  let (res_id3, progress) =
    Fact_acc.add_fact Predicate.(Hack FunctionDeclaration) json_key progress
  in
  let facts_function_declaration =
    extract_facts_exn "hack.FunctionDeclaration.6" (Fact_acc.to_json progress)
  in
  assert_bool
    "Identical keys for different predicates are separate facts"
    ((res_id :> int) != (res_id3 :> int));
  Int_asserter.assert_equals
    1
    (List.length facts_function_declaration)
    "One function decl fact added to JSON"

let test_add_decl_fact _test_ctxt =
  let progress = Fact_acc.init ~ownership:false in
  let gconst_name = "TestGConst" in
  let (id, prog) = Add_fact.gconst_decl gconst_name progress in
  let facts_global_const_declaration =
    extract_facts_exn "hack.GlobalConstDeclaration.6" (Fact_acc.to_json prog)
  in
  Int_asserter.assert_equals
    1
    (List.length facts_global_const_declaration)
    "One gconst fact added";
  let fact_json = List.nth_exn facts_global_const_declaration 0 in
  let fact_id = Jget.int_d (Some fact_json) "id" ~default:(-1) in
  let decl_name =
    return fact_json
    >>= get_obj "key"
    >>= get_obj "name"
    >>= get_obj "key"
    >>= get_obj "name"
    >>= get_string "key"
  in
  Int_asserter.assert_equals
    (id :> int)
    fact_id
    "Id returned is JSON id of new fact";
  match decl_name with
  | Ok (name, _) ->
    String_asserter.assert_equals gconst_name name "Nested fact contains name"
  | _ -> assert_failure "Could not extract decl name"

let test_build_xrefs _test_ctxt =
  let xrefs = XRefs.empty in
  Relative_path.set_path_prefix Relative_path.Root (Path.make "www");
  let file = Relative_path.from_root ~suffix:"test.php" in
  let decl_name = "TestDecl" in
  let target_json = JSON_Object [("declaration", JSON_String decl_name)] in
  let target_id = Fact_id.next () in
  let ref_pos =
    Pos.set_file
      file
      (Pos.make_from_lnum_bol_offset
         ~pos_file:file
         ~pos_start:(2, 5, 10)
         ~pos_end:(2, 5, 15))
  in
  let dup_ref_pos =
    Pos.set_file
      file
      (Pos.make_from_lnum_bol_offset
         ~pos_file:file
         ~pos_start:(2, 5, 10)
         ~pos_end:(2, 5, 15))
  in
  let next_ref_pos =
    Pos.set_file
      file
      (Pos.make_from_lnum_bol_offset
         ~pos_file:file
         ~pos_start:(3, 25, 40)
         ~pos_end:(3, 25, 45))
  in
  let target = XRefs.{ target = target_json; receiver_type = None } in
  let xrefs = XRefs.add xrefs target_id next_ref_pos target in
  let xrefs = XRefs.add xrefs target_id ref_pos target in
  let XRefs.{ fact_map; _ } = XRefs.add xrefs target_id dup_ref_pos target in
  let result =
    List.nth_exn (get_array_exn (Build_json.build_xrefs_json fact_map)) 0
  in
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
