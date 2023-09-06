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

let foo_name = "foo.php"

let foo_contents =
  Printf.sprintf "<?hh // strict

/* HH_FIXME[4336] */
function foo() : %s {
}
"

let foo_returns_int = foo_contents "int"

let foo_returns_string = foo_contents "string"

let bar_name = "bar.php"

let bar_contents = "<?hh // strict


function test(): int {
  return foo();
}
"

let bar_diagnostics =
  "
/bar.php:
File \"/bar.php\", line 5, characters 10-14:
Invalid return type (Typing[4110])
  File \"/bar.php\", line 4, characters 18-20:
  Expected `int`
  File \"/foo.php\", line 4, characters 18-23:
  But got `string`
"

let test () =
  let global_opts : GlobalOptions.t =
    GlobalOptions.set
      ~allowed_fixme_codes_strict:(ISet.of_list [4336])
      ~po_allowed_decl_fixme_codes:(ISet.of_list [4336])
      GlobalOptions.default
  in
  let custom_config = ServerConfig.default_config in
  let custom_config = ServerConfig.set_tc_options custom_config global_opts in
  let custom_config =
    ServerConfig.set_parser_options custom_config global_opts
  in
  Test.Client.with_env ~custom_config:(Some custom_config) @@ fun env ->
  let env =
    Test.Client.setup_disk
      env
      [(foo_name, foo_returns_int); (bar_name, bar_contents)]
  in

  (* Bar is clean with respect to foo as it is on disk *)
  let (env, diagnostics) = Test.Client.open_file env bar_name in
  Test.Client.assert_no_diagnostics diagnostics;

  (* If we make an in-editor change to foo, that won't yet affect bar *)
  let (env, _diagnostics) =
    Test.Client.edit_file env foo_name foo_returns_string
  in
  let (env, diagnostics) = Test.Client.open_file env bar_name in
  Test.Client.assert_no_diagnostics diagnostics;

  (* If we save the changes to foo, then bar will now exhibit the error *)
  let env = Test.Client.setup_disk env [(foo_name, foo_returns_string)] in
  let (env, diagnostics) = Test.Client.open_file env bar_name in
  Test.Client.assert_diagnostics_string diagnostics bar_diagnostics;

  (* And if we fix foo, then bar will now be correct again *)
  let env = Test.Client.setup_disk env [(foo_name, foo_returns_int)] in
  let (env, diagnostics) = Test.Client.open_file env bar_name in
  Test.Client.assert_no_diagnostics diagnostics;

  ignore env;
  ()
