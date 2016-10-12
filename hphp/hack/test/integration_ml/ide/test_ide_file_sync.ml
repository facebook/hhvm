(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Integration_test_base_types
open ServerCommandTypes

module Test = Integration_test_base

let foo_name = "foo.php"

let foo_no_errors = "<?hh
function foo() {

}
"

let foo_with_errors = "<?hh
{"

let bar_name = "bar.php"

let foo2_definition = "function foo2() {}\n"

let bar_contents = "<?hh
function test() {
  fo
}
"

let build_code_edit st_line st_column ed_line ed_column text =
  File_content.{
    range = Some {
      st = {
        line = st_line;
        column = st_column;
      };
      ed = {
        line = ed_line;
        column = ed_column;
      };
    };
    text;
  }

let () =

  let env = Test.setup_server () in
  let env = Test.setup_disk env [
    foo_name, foo_no_errors
  ] in
  let env = Test.connect_persistent_client env in

  (* Open a new file in editor *)
  let env = Test.open_file env bar_name ~contents:"" in
  (* Start typing in the new file *)
  let env, _ = Test.edit_file env bar_name bar_contents in
  (* Request completions *)
  let env, loop_output = Test.ide_autocomplete env (bar_name, 3, 5) in
  Test.assert_autocomplete loop_output ["foo"];

  (* Add a new definition to the file *)
  let env, _ = Test.(run_loop_once env { default_loop_input with
    persistent_client_request = Some (EDIT_FILE
      (Test.prepend_root bar_name, [build_code_edit 2 1 2 1 foo2_definition])
    )
  }) in

  (* Check that new definition is among the completions *)
  let _, loop_output = Test.ide_autocomplete env (bar_name, 4, 5) in
  Test.assert_autocomplete loop_output ["foo"; "foo2"]
