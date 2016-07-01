(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module SourceText = Full_fidelity_source_text
module SyntaxTree = Full_fidelity_syntax_tree
module PositionedSyntax = Full_fidelity_positioned_syntax
module ParserErrors = Full_fidelity_parser_errors
module SyntaxError = Full_fidelity_syntax_error
module TestUtils = Full_fidelity_test_utils

open Core
open OUnit

let test_files_dir = "./hphp/hack/test/full_fidelity/cases"

type test_case = {
  (** Source files is loaded from <name>.php in the <cwd>/<test_files_dir>/ *)
  name: string;
  source: string;
  expected: string;
  test_function: string -> string;
}

let ident str = str

let cat_file name =
  let path = Filename.concat test_files_dir name in
  let raw = Sys_utils.cat path in
  (** cat adds an extra newline at the end. *)
  if (String.length raw > 0) &&
      (String.get raw (String.length raw - 1)) == '\n' then
    String.sub raw 0 (String.length raw - 1)
  else
    raw

(** Create a test_case by reading input from <cwd>/<test_files_dir>/name.php
 * and name.exp *)
let make_test_case_from_files
    ?preprocess_exp:(preprocess_exp=ident) name test_function =
  let source = cat_file (name ^ ".php") in
  let expected = preprocess_exp (cat_file (name ^ ".exp")) in
  {
    name = name;
    source = source;
    expected = expected;
    test_function = test_function;
  }


let remove_whitespace text =
  let length = String.length text in
  let buffer = Buffer.create length in
  let rec aux i =
    if i = length then
      Buffer.contents buffer
    else
      let ch = String.get text i in
      match ch with
      | ' ' | '\n' | '\r' | '\t' -> aux (i + 1)
      | _ -> begin Buffer.add_char buffer ch; aux (i + 1) end in
  aux 0


let test_minimal source =
  let source_text = SourceText.make source in
  let syntax_tree = SyntaxTree.make source_text in
  TestUtils.minimal_to_string (SyntaxTree.root syntax_tree)

let test_mode source =
  let source_text = SourceText.make source in
  let syntax_tree = SyntaxTree.make source_text in
  let lang = SyntaxTree.language syntax_tree in
  let mode = SyntaxTree.mode syntax_tree in
  let is_strict = SyntaxTree.is_strict syntax_tree in
  let is_hack = SyntaxTree.is_hack syntax_tree in
  let is_php = SyntaxTree.is_php syntax_tree in
  Printf.sprintf "Lang:%sMode:%sStrict:%bHack:%bPhp:%b"
    lang mode is_strict is_hack is_php

let test_errors source =
  let source_text = SourceText.make source in
  let offset_to_position = SourceText.offset_to_position source_text in
  let syntax_tree = SyntaxTree.make source_text in
  let is_strict = SyntaxTree.is_strict syntax_tree in
  let root = PositionedSyntax.from_tree syntax_tree in
  let errors = ParserErrors.find_syntax_errors root is_strict in
  let mapper err = SyntaxError.to_positioned_string err offset_to_position in
  let errors = List.map errors ~f:mapper in
  Printf.sprintf "%s" (String.concat "\n" errors)


let test_data = [
  make_test_case_from_files
    ~preprocess_exp:remove_whitespace "test_simple" test_minimal;
  make_test_case_from_files
    ~preprocess_exp:remove_whitespace "test_conditional" test_minimal;
  make_test_case_from_files
    ~preprocess_exp:remove_whitespace "test_statements" test_minimal;
  make_test_case_from_files
    ~preprocess_exp:remove_whitespace "test_for_statements" test_minimal;
  make_test_case_from_files
    ~preprocess_exp:remove_whitespace "test_try_statement" test_minimal;
  make_test_case_from_files
    ~preprocess_exp:remove_whitespace "test_list_expression" test_minimal;
  make_test_case_from_files
    ~preprocess_exp:remove_whitespace "test_list_precedence" test_minimal;
  make_test_case_from_files
    ~preprocess_exp:remove_whitespace "test_foreach_statements" test_minimal;
  make_test_case_from_files
    ~preprocess_exp:remove_whitespace "test_types" test_minimal;
  make_test_case_from_files
    ~preprocess_exp:remove_whitespace "test_types_type_const" test_minimal;
  make_test_case_from_files
    ~preprocess_exp:remove_whitespace "test_object_creation" test_minimal;
  make_test_case_from_files
    ~preprocess_exp:remove_whitespace "test_function_call" test_minimal;
  make_test_case_from_files
    ~preprocess_exp:remove_whitespace "test_array_expression" test_minimal;
  make_test_case_from_files
    ~preprocess_exp:remove_whitespace "test_attribute_spec" test_minimal;
  make_test_case_from_files
    ~preprocess_exp:remove_whitespace "test_array_key_value_precedence" test_minimal;
  {
    name = "test_mode_1";
    source = "<?hh   ";
    expected = "Lang:hhMode:Strict:falseHack:truePhp:false";
    test_function = test_mode;
  };
  {
    name = "test_mode_2";
    source = "";
    expected = "Lang:Mode:Strict:falseHack:falsePhp:false";
    test_function = test_mode;
  };
  {
    name = "test_mode_3";
    source = "<?hh // strict ";
    expected = "Lang:hhMode:strictStrict:trueHack:truePhp:false";
    test_function = test_mode;
  };
  {
    name = "test_mode_4";
    source = "<?php // strict "; (* Not strict! *)
    expected = "Lang:phpMode:strictStrict:falseHack:falsePhp:true";
    test_function = test_mode;
  };
  {
    name = "test_mode_5";
    source = "<?hh/";
    expected = "Lang:hhMode:Strict:falseHack:truePhp:false";
    test_function = test_mode;
  };
  {
    name = "test_mode_6";
    source = "<?hh//";
    expected = "Lang:hhMode:Strict:falseHack:truePhp:false";
    test_function = test_mode;
  };
  make_test_case_from_files "test_errors_not_strict" test_errors;
  make_test_case_from_files "test_errors_strict" test_errors;
  make_test_case_from_files "test_no_errors_strict" test_errors;
  make_test_case_from_files
    ~preprocess_exp:remove_whitespace "test_empty_class" test_minimal;
]

let driver test () =
  let actual = test.test_function test.source in
  assert_equal test.expected actual

let run_test test =
  test.name >:: (driver test)

let run_tests tests =
  Printf.printf "%s" (Sys.getcwd());
  List.map tests ~f:run_test

let test_suite =
  "Full_fidelity_suite" >::: (run_tests test_data)

let main () =
  run_test_tt_main test_suite

let _ = main ()
