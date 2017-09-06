(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module SyntaxError = Full_fidelity_syntax_error
module SyntaxTree = Full_fidelity_syntax_tree
module SourceText = Full_fidelity_source_text
module TestUtils = Full_fidelity_test_utils
module Config = Random_ast_generator_config

(* open Core *)
open OUnit

type test = {source : string; parse : string; error : string}

let test_number = 1000
let test_count = 500

let gen_test count config =
  let module AstGenGrammar = Hack_grammar_descriptor in
  let module AstGen = Random_ast_generator.Make(AstGenGrammar) in
  let source = AstGen.generate count config in
  let file_path = Relative_path.(create Dummy "<gen_test>") in
  let gen_source = SourceText.make file_path source in
  let printer acc err =
    let error_message = SyntaxError.to_positioned_string err
      (SourceText.offset_to_position gen_source) in
    if acc = "" then error_message
    else Printf.sprintf "%s, %s" error_message acc in
  let syntax_tree = SyntaxTree.make gen_source in
  let gen_errors = SyntaxTree.errors syntax_tree in
  let parse = TestUtils.minimal_to_string (SyntaxTree.root syntax_tree) in
  let error = List.fold_left printer "" gen_errors in
  {source; parse; error}



let test_data config =
  let config = Config.find_config config in
  let rec gen num acc =
    if num >= test_number then acc
    else let test = gen_test test_count config in
      gen (num + 1) (test :: acc)
  in
  gen 0 []

let driver test () =
  assert_equal "" test.error

let run_test test =
  let name = Printf.sprintf "Source: %s\nParse: %s" test.source test.parse in
  name >:: (driver test)

let run_tests tests =
  Printf.printf "%s" (Sys.getcwd());
  List.map run_test tests

let test_suite config =
  let lst = test_data config in
  let len = string_of_int (List.length lst) in
  "Full_fidelity_random_suite"^len >::: (run_tests (test_data config))

let main () =
  run_test_tt_main (test_suite "LongAssignmentConfig")

let _ = main ()
