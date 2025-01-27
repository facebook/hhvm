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
  let po =
    ParserOptions.
      {
        default with
        auto_namespace_map = [("ShortName", "HH\\LongName\\ShortName")];
        deregister_php_stdlib = true;
      }
  in
  let global_opts : GlobalOptions.t =
    GlobalOptions.set
      ~po
      ~tco_saved_state:GlobalOptions.default_saved_state
      GlobalOptions.default
  in
  let custom_config = ServerConfig.default_config in
  let custom_config = ServerConfig.set_tc_options custom_config global_opts in
  let custom_config = ServerConfig.set_parser_options custom_config po in
  Test.Client.with_env ~custom_config:(Some custom_config) @@ fun env ->
  let env = Test.Client.setup_disk env [("foo.php", foo_contents)] in
  let env =
    let get_name i = "test" ^ string_of_int i ^ ".php" in
    Test.Client.setup_disk env
    @@ List.mapi
         (fun i contents ->
           let clean_contents =
             Str.global_replace (Str.regexp_string "AUTO332") "" contents
           in
           (get_name i, clean_contents))
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
  let test_ide env contents i expected =
    let path = "test" ^ string_of_int i ^ ".php" in
    let offset =
      String_utils.substring_index AutocompleteTypes.autocomplete_token contents
    in
    let clean_contents =
      Str.global_replace (Str.regexp_string "AUTO332") "" contents
    in
    let position = File_content.offset_to_position contents offset in
    let (line, column) = File_content.Position.line_column_one_based position in
    let (env, _diagnostics) = Test.Client.open_file env path in
    let (env, response) =
      ClientIdeDaemon.Test.handle
        env
        ClientIdeMessage.(
          Completion
            ( Test.doc path clean_contents,
              Test.loc line column,
              { is_manually_invoked = true } ))
    in
    Test.assert_ide_completions response expected;
    ignore env;
    ()
  in
  (* Note that autocomplete now hides namespaces when you've already typed them!
   * This means that all tests will simply return "foo" as long as you're in
   * the correct namespace when autocomplete is triggered. *)
  test_ide env autocomplete_contents0 0 ["foo"];
  test_ide env autocomplete_contents1 1 [];
  test_ide env autocomplete_contents2 2 ["foo"];
  test_ide env autocomplete_contents3 3 ["foo"];
  test_ide env autocomplete_contents4 4 [];
  test_ide env autocomplete_contents5 5 ["foo"];
  test_ide env autocomplete_contents6 6 ["foo"];
  test_ide env autocomplete_contents7 7 [];
  ()
