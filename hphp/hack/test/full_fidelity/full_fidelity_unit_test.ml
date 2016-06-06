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
module SyntaxKind = Full_fidelity_syntax_kind
module TriviaKind = Full_fidelity_trivia_kind
module TokenKind = Full_fidelity_token_kind
module MinimalSyntax = Full_fidelity_minimal_syntax
module MinimalToken = Full_fidelity_minimal_token
module MinimalTrivia = Full_fidelity_minimal_trivia

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

let minimal_trivia_to_string trivia =
  let name = TriviaKind.to_string (MinimalTrivia.kind trivia) in
  Printf.sprintf "(%s)" name

let minimal_trivia_list_to_string trivia_list =
  String.concat "" (List.map trivia_list ~f:minimal_trivia_to_string)

let minimal_token_to_string token =
  let leading = minimal_trivia_list_to_string (MinimalToken.leading token) in
  let name = TokenKind.to_string (MinimalToken.kind token) in
  let name =
    if name = "(" then "lparen"
    else if name = ")" then "rparen"
    else name in
  let trailing = minimal_trivia_list_to_string (MinimalToken.trailing token) in
  Printf.sprintf "(%s(%s)%s)" leading name trailing

let rec minimal_to_string node =
  match MinimalSyntax.syntax node with
  | MinimalSyntax.Token token ->
    minimal_token_to_string token
  | _ ->
    let name = SyntaxKind.to_string (MinimalSyntax.kind node) in
    let children = MinimalSyntax.children node in
    let children = List.map children ~f:minimal_to_string in
    let children = String.concat "" children in
    Printf.sprintf "(%s%s)" name children

let test_minimal source =
  let source_text = SourceText.make source in
  let syntax_tree = SyntaxTree.make source_text in
  minimal_to_string (SyntaxTree.root syntax_tree)

let source_simple =
"<?hh
/* comment */ function foo() {
  $a = (123 + $b) * $c;
}"

let result_simple = remove_whitespace
  "(script
    (header((<))((?))((name)(end_of_line)))
    (list
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
          (list
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
              ((;)(end_of_line))))
          ((}))))))"

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
  (list
    (function_declaration(missing)(missing)((function)(whitespace))((name))
    (missing)((lparen))(missing)((rparen)(whitespace))(missing)(missing)
    (compound_statement
      (({)(end_of_line))
        (list
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
            (list
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
                  ((;)(end_of_line)))))
            (missing)))
      ((}))))))"

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
