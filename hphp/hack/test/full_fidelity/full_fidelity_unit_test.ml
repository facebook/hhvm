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

type code_extent_test = {
  name: string;
  source: string;
  expected: string;
  test_function: string -> string;
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

let source_simple =
"<?hh
/* comment */ function foo() {
  $a = (123 + $b) * $c;
}"

let result_simple = remove_whitespace
  "(script
    (header((<))((?))((name)(end_of_line)))
      (function_declaration
        (missing)
        (missing)
        ((delimited_comment)(whitespace)(function)(whitespace))
        ((name))
        (missing)
        ((lparen))
        (missing)
        ((rparen)(whitespace))
        (missing)
        (missing)
        (compound_statement
          (({)(end_of_line))
            (expression_statement
              (binary_operator
                (variable((whitespace)(variable)(whitespace)))
                ((=)(whitespace))
                (binary_operator
                  (parenthesized_expression
                    ((lparen))
                    (binary_operator
                      (literal((decimal_literal)(whitespace)))
                      ((+)(whitespace))
                      (variable((variable))))
                    ((rparen)(whitespace)))
                  ((*)(whitespace))
                  (variable((variable)))))
              ((;)(end_of_line)))
          ((})))))"

let source_statements =
"<?hh
function foo() {
  if ($a)
    if ($b)
      switch ($c) {
        case 123: break;
        default: break;
      }
    else
      return $d;
  elseif($e)
    do {
      while($f)
        throw $g;
      continue;
    } while ($h);
}"

let result_statements = remove_whitespace "
(script(header((<))((?))((name)(end_of_line)))
    (function_declaration(missing)(missing)((function)(whitespace))((name))
    (missing)((lparen))(missing)((rparen)(whitespace))(missing)(missing)
    (compound_statement
      (({)(end_of_line))
          (if_statement
            ((whitespace)(if)(whitespace))
            ((lparen))(variable((variable)))
            ((rparen)(end_of_line))
            (if_statement
              ((whitespace)(if)(whitespace))
              ((lparen))
              (variable((variable)))
              ((rparen)(end_of_line))
              (switch_statement
                ((whitespace)(switch)(whitespace))
                ((lparen))
                (variable((variable)))
                ((rparen)(whitespace))
                (compound_statement
                  (({)(end_of_line))
                    (list
                      (case_statement
                        ((whitespace)(case)(whitespace))
                        (literal((decimal_literal)))
                        ((:)(whitespace))
                        (break_statement((break))((;)(end_of_line))))
                      (default_statement
                        ((whitespace)(default))
                        ((:)(whitespace))
                        (break_statement((break))((;)(end_of_line)))))
                    ((whitespace)(})(end_of_line))))
              (missing)
              (else_clause
                ((whitespace)(else)(end_of_line))
                (return_statement
                  ((whitespace)(return)(whitespace))
                  (variable((variable)))
                  ((;)(end_of_line)))))
              (elseif_clause
                ((whitespace)(elseif))
                ((lparen))
                (variable((variable)))
                ((rparen)(end_of_line))
                (do_statement
                  ((whitespace)(do)(whitespace))
                  (compound_statement
                    (({)(end_of_line))
                    (list
                      (while_statement
                        ((whitespace)(while))
                        ((lparen))
                        (variable((variable)))
                        ((rparen)(end_of_line))
                        (throw_statement
                          ((whitespace)(throw)(whitespace))
                          (variable((variable)))
                          ((;)(end_of_line))))
                      (continue_statement
                        ((whitespace)(continue))
                        ((;)(end_of_line))))
                    ((whitespace)(})(whitespace)))
                  ((while)(whitespace))
                  ((lparen))
                  (variable((variable)))
                  ((rparen))
                  ((;)(end_of_line))))
            (missing))
      ((})))))"

let source_for_statements =
"<?hh
function foo() {
  for($x;$y;$z) {
    return $a;
  }
  for($x;$y;$z) {}
  for(;$y;$z) {}
  for($x;;$z) {}
  for($x;$y;) {}
  for($x;;) {}
  for(;$y;) {}
  for(;;$z) {}
  for($x, $y;;) {}
  for(;$x, $y, $z;) {}
}"

let result_for_statements = remove_whitespace
"(script(header((<))((?))((name)(end_of_line)))(function_declaration
  (missing)
  (missing)
  ((function)(whitespace))
  ((name))
  (missing)
  ((lparen))
  (missing)
  ((rparen)(whitespace))
  (missing)
  (missing)
  (compound_statement(({)(end_of_line))(list
    (for_statement
      ((whitespace)(for))
      ((lparen))
      (variable((variable)))
      ((;))
      (variable((variable)))
      ((;))
      (variable((variable)))
      ((rparen)(whitespace))
      (compound_statement
        (({)(end_of_line))
        (return_statement
          ((whitespace)(return)(whitespace))
          (variable((variable)))
          ((;)(end_of_line))
        )
        ((whitespace)(})(end_of_line))
      )
    )
    (for_statement
      ((whitespace)(for))
      ((lparen))
      (variable((variable)))
      ((;))
      (variable((variable)))
      ((;))
      (variable((variable)))
      ((rparen)(whitespace))
      (compound_statement(({))(missing)((})(end_of_line)))
    )
    (for_statement
      ((whitespace)(for))
      ((lparen))
      (missing)
      ((;))
      (variable((variable)))
      ((;))
      (variable((variable)))
      ((rparen)(whitespace))
      (compound_statement(({))(missing)((})(end_of_line)))
    )
    (for_statement
      ((whitespace)(for))
      ((lparen))
      (variable((variable)))
      ((;))
      (missing)
      ((;))
      (variable((variable)))
      ((rparen)(whitespace))
      (compound_statement(({))(missing)((})(end_of_line)))
    )
    (for_statement
      ((whitespace)(for))
      ((lparen))
      (variable((variable)))
      ((;))
      (variable((variable)))
      ((;))
      (missing)
      ((rparen)(whitespace))
      (compound_statement(({))(missing)((})(end_of_line)))
    )
    (for_statement
      ((whitespace)(for))
      ((lparen))
      (variable((variable)))
      ((;))
      (missing)
      ((;))
      (missing)
      ((rparen)(whitespace))
      (compound_statement(({))(missing)((})(end_of_line)))
    )
    (for_statement
      ((whitespace)(for))
      ((lparen))
      (missing)
      ((;))
      (variable((variable)))
      ((;))
      (missing)
      ((rparen)(whitespace))
      (compound_statement(({))(missing)((})(end_of_line)))
    )
    (for_statement
      ((whitespace)(for))
      ((lparen))
      (missing)
      ((;))
      (missing)
      ((;))
      (variable((variable)))
      ((rparen)(whitespace))
      (compound_statement(({))(missing)((})(end_of_line)))
    )
    (for_statement
      ((whitespace)(for))
      ((lparen))
      (list(variable((variable)))((,)(whitespace))(variable((variable))))
      ((;))
      (missing)
      ((;))
      (missing)
      ((rparen)(whitespace))
      (compound_statement(({))(missing)((})(end_of_line)))
    )
    (for_statement
      ((whitespace)(for))
      ((lparen))
      (missing)
      ((;))
      (list
        (variable((variable)))
        ((,)(whitespace))
        (variable((variable)))
        ((,)(whitespace))
        (variable((variable)))
      )
      ((;))
      (missing)
      ((rparen)(whitespace))
      (compound_statement(({))(missing)((})(end_of_line)))
    )
  )((})))
))"

let source_errors_strict =
"<?hh // strict
function foo($a) {
  return $a;
}"

let source_errors_not_strict =
"<?hh
function foo($a) {
  return $a;
}"

let source_no_errors_strict =
"<?hh // strict
function foo(int $a) : int {
  return $a;
}"

let results_errors_strict =
"(2,14)-(2,15) A type annotation is required in strict mode.
(2,16)-(2,16) A type annotation is required in strict mode."

let test_data = [
  {
    name = "test_simple";
    source = source_simple;
    expected = result_simple;
    test_function = test_minimal;
  };
  {
    name = "test_statements";
    source = source_statements;
    expected = result_statements;
    test_function = test_minimal;
  };
  {
    name = "test_for_statements";
    source = source_for_statements;
    expected = result_for_statements;
    test_function = test_minimal;
  };
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
  {
    name = "test_errors_not_strict";
    source = source_errors_not_strict;
    expected = "";
    test_function = test_errors;
  };
  {
    name = "test_errors_strict";
    source = source_errors_strict;
    expected = results_errors_strict;
    test_function = test_errors;
  };
  {
    name = "test_no_errors_strict";
    source = source_no_errors_strict;
    expected = "";
    test_function = test_errors;
  };

]

let driver test () =
  let actual = test.test_function test.source in
  assert_equal test.expected actual

let run_test test =
  test.name >:: (driver test)

let run_tests tests =
  List.map tests ~f:run_test

let test_suite =
  "Full_fidelity_suite" >::: (run_tests test_data)

let main () =
  run_test_tt_main test_suite

let _ = main ()
