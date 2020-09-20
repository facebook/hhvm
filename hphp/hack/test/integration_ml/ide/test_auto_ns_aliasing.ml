(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

module Test = Integration_test_base

let foo_contents =
  "<?hh // strict

namespace HH\\LongName\\ShortName;

function foo() : void {}
"

let autocomplete_contents0 =
  "<?hh // strict

function testTypecheck(): void {
  ShortName\\foAUTO332;
}
"

let autocomplete_contents1 =
  "<?hh

function testTypecheck(): void {
  \\ShortName\\foAUTO332;
}
"

let autocomplete_contents2 =
  "<?hh // strict

function testTypecheck(): void {
  HH\\LongName\\ShortName\\foAUTO332;
}
"

let autocomplete_contents3 =
  "<?hh // strict

function testTypecheck(): void {
  \\HH\\LongName\\ShortName\\foAUTO332;
}
"

let autocomplete_contents4 =
  "<?hh // strict

namespace Test;

function testTypecheck(): void {
  HH\\LongName\\ShortName\\foAUTO332;
}
"

let autocomplete_contents5 =
  "<?hh // strict

namespace Test;

function testTypecheck(): void {
  \\HH\\LongName\\ShortName\\foAUTO332;
}
"

let autocomplete_contents6 =
  "<?hh // strict

namespace Test;

function testTypecheck(): void {
  ShortName\\foAUTO332;
}
"

let autocomplete_contents7 =
  "<?hh // strict

namespace Test;

function testTypecheck(): void {
  \\ShortName\\foAUTO332;
}
"

let test () =
  let global_opts =
    GlobalOptions.make
      ~po_auto_namespace_map:[("ShortName", "HH\\LongName\\ShortName")]
      ~po_deregister_php_stdlib:true
      ~po_allow_goto:false
      ~tco_unsafe_rx:false
      ()
  in
  let custom_config = ServerConfig.default_config in
  let custom_config = ServerConfig.set_tc_options custom_config global_opts in
  let custom_config =
    ServerConfig.set_parser_options custom_config global_opts
  in
  let env = Test.setup_server ~custom_config () in
  let env = Test.setup_disk env [("foo.php", foo_contents)] in
  let env =
    let get_name i = "test" ^ string_of_int i ^ ".php" in
    Test.setup_disk env
    @@ List.mapi
         (fun i contents -> (get_name i, contents))
         [
           autocomplete_contents0;
           autocomplete_contents1;
           autocomplete_contents2;
           autocomplete_contents3;
           autocomplete_contents4;
           autocomplete_contents5;
           autocomplete_contents6;
           autocomplete_contents7;
         ]
  in
  let env = Test.connect_persistent_client env in
  let test_legacy env contents expected =
    let (_, loop_output) = Test.autocomplete env contents in
    Test.assert_autocomplete loop_output expected
  in
  let test_ide env contents i expected =
    let path = "test" ^ string_of_int i ^ ".php" in
    let offset =
      String_utils.substring_index AutocompleteTypes.autocomplete_token contents
    in
    let position = File_content.offset_to_position contents offset in
    let line = position.File_content.line - 1 in
    let column = position.File_content.column - 1 in
    let (_, loop_output) = Test.ide_autocomplete env (path, line, column) in
    Test.assert_ide_autocomplete loop_output expected
  in
  (* Note that autocomplete now hides namespaces when you've already typed them!
   * This means that all tests will simply return "foo" as long as you're in
   * the correct namespace when autocomplete is triggered. *)
  test_legacy env autocomplete_contents0 ["foo"];
  test_legacy env autocomplete_contents1 [""];
  test_legacy env autocomplete_contents2 ["foo"];
  test_legacy env autocomplete_contents3 ["foo"];
  test_legacy env autocomplete_contents4 [""];
  test_legacy env autocomplete_contents5 ["foo"];
  test_legacy env autocomplete_contents6 ["foo"];
  test_legacy env autocomplete_contents7 [""];

  test_ide env autocomplete_contents0 0 ["foo"];
  test_ide env autocomplete_contents1 1 [];
  test_ide env autocomplete_contents2 2 ["foo"];
  test_ide env autocomplete_contents3 3 ["foo"];
  test_ide env autocomplete_contents4 4 [];
  test_ide env autocomplete_contents5 5 ["foo"];
  test_ide env autocomplete_contents6 6 ["foo"];
  test_ide env autocomplete_contents7 7 [];
  ()
