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
module Grammar = Hack_grammar_descriptor

(* open Core *)
open OUnit
open Ocaml_overrides

type test = {config : string; error : int}

module StrMap = Map.Make(struct
    type t = string
    let compare = Pervasives.compare
end)

let create_mapping () =
  let start = Grammar.start in
  let rec create_mapping_helper lhs map =
    let name = Grammar.nonterm_to_string lhs in
    if StrMap.mem name map then map
    else
      let rules = Grammar.grammar lhs in
      let num_rules = List.length rules in
      let map = StrMap.add name num_rules map in
      let list_fold_fun acc el = match el with
        | Grammar.NonTerm nonterm -> create_mapping_helper nonterm acc
        | Grammar.Term _ -> acc
      in
      let list_list_fold_fun acc el =
        List.fold_left list_fold_fun acc el
      in
      List.fold_left list_list_fold_fun map rules
  in
  create_mapping_helper start StrMap.empty

(* return a string as error message *)
let check_config map config =
  let map_fun (name, lst) =
    let sum = List.fold_left (+.) 0. lst in
    let length = List.length lst in
    if not (StrMap.mem name map) then
      Printf.sprintf "Non-terminal %s not found in the grammar." name
    else
      let target_length = StrMap.find name map in
      if sum <> 1. then
        Printf.sprintf "%s: The sum of weight is not 1." name
      else if target_length <> length then
        Printf.sprintf "%s: Expected number of rules is %d, but got %d."
          name target_length length
      else ""
  in
  let error_list = List.map map_fun config in
  let concat_fun acc el = match acc, el with
    | "", x
    | x, "" -> x
    | x, y -> Printf.sprintf "%s\n%s" x y
  in
  List.fold_left concat_fun "" error_list


let driver test map () =
  let error = check_config map test in
  assert_equal "" error

let run_test map (name, test) =
  name >:: (driver test map)

let run_tests tests =
  Printf.printf "%s" (Sys.getcwd());
  let map = create_mapping () in
  List.map (run_test map) tests

let test_suite =
  "Full_fidelity_random_ast_config" >::: (run_tests Config.mapping)

let main () =
  run_test_tt_main test_suite

let _ = main ()
