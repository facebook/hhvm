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
  Printf.sprintf
    "<?hh // strict
%s
/* HH_FIXME[4336] */
function foo(): string {
}"

let bar_name = Printf.sprintf "bar%d.php"

let bar_contents =
  Printf.sprintf "<?hh

function bar%d (): int {
  return foo();
}
"

let create_bar i = (bar_name i, bar_contents i)

let bar_106_name = bar_name 106

let bar_107_name = bar_name 107

let rec create_bars acc = function
  | 0 -> acc
  | i -> create_bars (create_bar i :: acc) (i - 1)

let bar_107_foo_line_3_diagnostics =
  {|
/bar107.php:
File "/bar107.php", line 4, characters 10-14:
Invalid return type (Typing[4110])
  File "/bar107.php", line 3, characters 21-23:
  Expected `int`
  File "/foo.php", line 4, characters 17-22:
  But got `string`
|}

let bar_107_foo_line_5_diagnostics =
  {|
/bar107.php:
File "/bar107.php", line 4, characters 10-14:
Invalid return type (Typing[4110])
  File "/bar107.php", line 3, characters 21-23:
  Expected `int`
  File "/foo.php", line 6, characters 17-22:
  But got `string`
|}

let bar106_cleared = {|
/bar106.php:
|}

let bar107_cleared = {|
/bar107.php:
|}

let bar_108_foo_line_5_diagnostics =
  {|
/bar108.php:
File "/bar108.php", line 4, characters 10-14:
Invalid return type (Typing[4110])
  File "/bar108.php", line 3, characters 21-23:
  Expected `int`
  File "/foo.php", line 6, characters 17-22:
  But got `string`
|}

let bar_109_foo_line_3_diagnostics =
  {|
/bar109.php:
File "/bar109.php", line 4, characters 10-14:
Invalid return type (Typing[4110])
  File "/bar109.php", line 3, characters 21-23:
  Expected `int`
  File "/foo.php", line 4, characters 17-22:
  But got `string`
|}

let root = "/"

let hhconfig_filename = Filename.concat root ".hhconfig"

let hhconfig_contents =
  "
allowed_fixme_codes_strict = 4336
allowed_decl_fixme_codes = 4336
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
  (* 200 files with errors *)
  let disk_contents = [(foo_name, foo_contents "")] in
  let disk_contents = create_bars disk_contents 200 in
  let env = Test.Client.setup_disk env disk_contents in
  let (env, diagnostics) = Test.Client.open_file env bar_107_name in
  Test.Client.assert_diagnostics_string
    diagnostics
    bar_107_foo_line_3_diagnostics;

  (* Move foo 2 lines down and save to disk *)
  let env = Test.Client.setup_disk env [(foo_name, foo_contents "\n\n")] in
  let (env, diagnostics) = Test.Client.open_file env bar_107_name in
  Test.Client.assert_diagnostics_string
    diagnostics
    bar_107_foo_line_5_diagnostics;
  let (env, diagnostics) = Test.Client.open_file env (bar_name 108) in
  Test.Client.assert_diagnostics_string
    diagnostics
    bar_108_foo_line_5_diagnostics;

  (* Fix one of the errors in the editor *)
  let (env, diagnostics) = Test.Client.edit_file env bar_107_name "" in
  Test.Client.assert_no_diagnostics diagnostics;
  ignore env;
  ()
