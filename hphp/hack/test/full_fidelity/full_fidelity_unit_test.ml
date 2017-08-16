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

let fix_output_text_header text =
  (* patches the output of the test to  *)
  (*TODO: will be removed in a diff that updates all test baselines *)
  let replacements = [
    "(script(list(markup_section(missing)((markup))(markup_suffix((<?))((name)\
      (end_of_line)))(missing))",
    "(script(header((<))((?))((name)(end_of_line)))(list";
    "(script(list(markup_section(missing)((markup))(markup_suffix((<?))((name)\
      (whitespace)(single_line_comment)(end_of_line)))(missing))",
    "(script(header((<))((?))((name)(whitespace)(single_line_comment)\
      (end_of_line)))(list"
  ] in
  let starts_with s = String_utils.string_starts_with text s in
  match List.find ~f:(fun (s, _) -> starts_with s) replacements with
  | Some (prefix, new_prefix) ->
    new_prefix ^ (String_utils.lstrip text prefix)
  | None -> text

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
  let is_hack = (SyntaxTree.language syntax_tree = "hh") in
  let root = PositionedSyntax.from_tree syntax_tree in
  let errors1 = SyntaxTree.errors syntax_tree in
  let errors2 = ParserErrors.find_syntax_errors root is_strict is_hack in
  let errors = errors1 @ errors2 in
  let mapper err = SyntaxError.to_positioned_string err offset_to_position in
  let errors = List.map errors ~f:mapper in
  Printf.sprintf "%s" (String.concat "\n" errors)


let minimal_tests =
  let mapper testname =
    make_test_case_from_files
      ~preprocess_exp:remove_whitespace testname test_minimal in
  List.map
  [
    "test_simple";
(*  TODO: This test is temporarily disabled because
    $a ? $b : $c = $d
    does not parse in the FF parser as it did in the original Hack parser,
    due to a precedence issue. Re-enable this test once we either fix that,
    or decide to take the breaking change.
    "test_conditional"; *)
    "test_statements";
    "test_for_statements";
    "test_try_statement";
    "test_list_precedence";
    "test_list_expression";
    "test_foreach_statements";
    "test_types_type_const";
    "test_function_call";
    "test_array_expression";
    "test_varray_darray_expressions";
    "test_varray_darray_types";
    "test_attribute_spec";
    "test_array_key_value_precedence";
    "test_enum";
    "test_class_with_attributes";
    "test_namespace";
    "test_empty_class";
    "test_class_method_declaration";
    "test_constructor_destructor";
    "test_trait";
    "test_type_const";
    "test_class_const";
    "test_type_alias";
    "test_indirection";
    "test_eval_deref";
    "test_global_constant";
    "test_closure_type";
    "test_inclusion_directive";
    "test_awaitable_creation";
    "test_literals";
    "test_variadic_type_hint";
    "test_tuple_type_keyword";
    "test_trailing_commas";
    "context/test_extra_error_trivia";
    "test_funcall_with_type_arguments";
  ] ~f:mapper

let error_tests =
  let mapper testname =
    make_test_case_from_files testname test_errors in
  List.map
  [
    "test_alias_errors";
    "test_errors_not_strict";
    "test_errors_strict";
    "test_no_errors_strict";
    "test_statement_errors";
    "test_expression_errors";
    "test_errors_method";
    "test_declaration_errors";
    "test_errors_class";
    "test_errors_array_type";
    "test_errors_variadic_param";
    "test_errors_statements";
    "test_implements_errors";
    "test_object_creation_errors";
    "test_classish_inside_function_errors";
    "test_list_expression_errors";
    "test_interface_method_errors";
    "test_abstract_classish_errors";
    "test_abstract_methodish_errors";
    "test_async_errors";
    "test_visibility_modifier_errors";
    "test_legal_php";
    "context/test_missing_name_in_expression";
    "context/test_nested_function_lite";
    "context/test_nested_function";
    "context/test_method_decl_extra_token";
    "context/test_recovery_to_classish1";
    "context/test_recovery_to_classish2";
    "context/test_recovery_to_classish3";
    "context/test_single_extra_token_recovery";
    "context/test_missing_foreach_value";
    "test_namespace_error_recovery";
    "test_correct_code1";
  ] ~f:mapper

let test_data = minimal_tests @ error_tests @
[
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
  }
]

let driver test () =
  let actual = test.test_function test.source in
  let actual = fix_output_text_header actual in
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
