(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

open Reordered_argument_collections
module Test = Integration_test_base

let foo_name = "foo.php"

let foo_contents =
  Printf.sprintf "<?hh
/* HH_FIXME[4336] */
function foo(): %s {

}
"

let foo_returns_int = foo_contents "int"

let foo_returns_string = foo_contents "string"

let bar_name = Printf.sprintf "bar%d.php"

let bar_contents =
  Printf.sprintf "<?hh

function bar%d (): int {
  return foo();
}
"

let baz_name = "baz.php"

let baz_contents = "<?hh

{
"

let create_bar i = (bar_name i, bar_contents i)

let get_diagnostics_map = Integration_test_base.get_diagnostics

let get_files_with_errors diagnostics_map =
  SSet.of_list (SMap.keys diagnostics_map)

let rec create_bars acc = function
  | 0 -> acc
  | i -> create_bars (create_bar i :: acc) (i - 1)

let bar_10_clear_diagnostics = "
/bar10.php:
"

let bar_106_diagnostics =
  {|
/bar106.php:
File "/bar106.php", line 4, characters 10-14:
Invalid return type (Typing[4110])
  File "/bar106.php", line 3, characters 21-23:
  Expected `int`
  File "/foo.php", line 3, characters 17-22:
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
  Relative_path.set_path_prefix Relative_path.Root (Path.make root);
  TestDisk.set hhconfig_filename hhconfig_contents;
  let hhconfig_path =
    Relative_path.create Relative_path.Root hhconfig_filename
  in
  let options = ServerArgs.default_options ~root in
  let (custom_config, _) =
    ServerConfig.load ~silent:false hhconfig_path options
  in
  let env = Test.setup_server ~custom_config () in
  let disk_contests = [(foo_name, foo_returns_int)] in
  let disk_contests = create_bars disk_contests 200 in
  let env = Test.setup_disk env disk_contests in
  let env = Test.connect_persistent_client env in
  let env = Test.subscribe_diagnostic env ~error_limit:10 in
  let (env, loop_output) = Test.(run_loop_once env default_loop_input) in
  Test.assert_no_errors env;
  Test.assert_no_diagnostics loop_output;

  (* Change a definition that introduces error in all 200 files, and also
   * make another error in other open file *)
  let env = Test.open_file env foo_name ~contents:foo_returns_string in
  let env = Test.open_file env baz_name ~contents:baz_contents in
  let env = Test.wait env in
  let (env, loop_output) = Test.(run_loop_once env default_loop_input) in
  (* Make sure that the open file is among the errors *)
  let diagnostics_map = get_diagnostics_map loop_output in
  let files_with_errors = get_files_with_errors diagnostics_map in
  let baz_name = Test.prepend_root baz_name in
  if not @@ SSet.mem files_with_errors baz_name then
    Test.fail "Expected diagnostics for baz.php";

  (* Trigger global recheck *)
  let (env, loop_output) = Test.full_check_status env in
  let diagnostics_map = get_diagnostics_map loop_output in
  let files_with_errors = get_files_with_errors diagnostics_map in
  SSet.iter files_with_errors ~f:print_endline;

  let error_count =
    SMap.fold diagnostics_map ~init:0 ~f:(fun _key errors count ->
        count + List.length errors)
  in
  if error_count > 10 then
    Test.fail
    @@ Printf.sprintf "Expected no more than 10 errors but got %d." error_count;

  (* Fix one of the remaining errors *)
  let bar_10_name = bar_name 10 in
  let (env, _) = Test.edit_file env bar_10_name "" in
  let env = Test.wait env in
  let (env, loop_output) = Test.(run_loop_once env default_loop_input) in
  (* Check that the errors from bar10 are removed *)
  Test.assert_diagnostics loop_output bar_10_clear_diagnostics;

  (* Trigger another global recheck to get more global errors *)
  let (_, loop_output) = Test.full_check_status env in
  Test.assert_diagnostics loop_output bar_106_diagnostics
