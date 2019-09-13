(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

module Test = Integration_test_base

let foo_name = "foo.php"

let foo_no_errors = "<?hh
function foo() {

}
"

let foo_with_errors = "<?hh
{"

let bar_name = "bar.php"

let bar_contents = "<?hh
function test() {
  fo
}
"

let bar_new_contents = "<?hh
function foo2() {}
function test() {
  fo
}
"

let build_code_edit st_line st_column ed_line ed_column text =
  Ide_api_types.
    {
      range =
        Some
          {
            st = { line = st_line; column = st_column };
            ed = { line = ed_line; column = ed_column };
          };
      text;
    }

let test () =
  let env = Test.setup_server () in
  let env = Test.setup_disk env [(foo_name, foo_no_errors)] in
  let env = Test.connect_persistent_client env in
  (* Open a new file in editor *)
  let env = Test.open_file env bar_name ~contents:"" in
  (* Start typing in the new file *)
  let (env, _) = Test.edit_file env bar_name bar_contents in
  (* Request completions *)
  let (env, loop_output) = Test.ide_autocomplete env (bar_name, 3, 5) in
  Test.assert_ide_autocomplete loop_output ["foo"];

  (* Add a new definition to the file and save it *)
  let (env, _) = Test.edit_file env bar_name bar_new_contents in
  let (env, _) = Test.save_file env bar_name bar_new_contents in
  (* Check that new definition is among the completions *)
  let (_, loop_output) = Test.ide_autocomplete env (bar_name, 4, 5) in
  Test.assert_ide_autocomplete loop_output ["foo"; "foo2"]
