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

let foo_contents = "<?hh
class C {
  public function foo() {

  }
}
"

let foo_contents_with_parse_error = "<?hh
class C {
  public function bar() {PARSE_ERROR

  }
}
"

let bar_name = "bar.php"

let bar_contents = "<?hh
function test() {
  (new C())->f
}
"

let run_and_check_autocomplete env =
  (* Simulate time passing to trigger recheck recheck *)
  let env = ServerEnv.{ env with last_command_time = 0.0 } in

  let env, loop_output = Test.(run_loop_once env { default_loop_input with
    persistent_client_request = Some (IDE_AUTOCOMPLETE
      (bar_name, File_content.{line = 3; column = 15})
    )
  }) in
  (match loop_output.persistent_client_response with
  | Some [x] when x.AutocompleteService.res_name = "foo" -> ()
  | _ -> Test.fail "Unexpected or missing autocomplete response");
  env

let () =

  let env = Test.setup_server () in
  let env = Test.connect_persistent_client env in

  (* Create and put content in two files *)
  let env, _ = Test.(run_loop_once env { default_loop_input with
    persistent_client_request = Some (OPEN_FILE foo_name)
  }) in
  let env, _ = Test.(run_loop_once env { default_loop_input with
    persistent_client_request = Some (OPEN_FILE bar_name)
  }) in
  let env, _ = Test.(run_loop_once env { default_loop_input with
    persistent_client_request = Some (EDIT_FILE
      (foo_name, [File_content.{range = None; text = foo_contents;}])
    )
  }) in
  let env, _ = Test.(run_loop_once env { default_loop_input with
    persistent_client_request = Some (EDIT_FILE
      (bar_name, [File_content.{range = None; text = bar_contents;}])
    )
  }) in

  (* Check that autocompletions in one file are aware of definitions in
   * another one*)
  let env = run_and_check_autocomplete env in

  let env, _ = Test.(run_loop_once env { default_loop_input with
    persistent_client_request = Some (EDIT_FILE
      (foo_name, [File_content.{
        range = None; text = foo_contents_with_parse_error;}])
    )
  }) in
  (* If C had parse errors, we'll not update it's declarations, so
   * the result will not change *)
  let _ = run_and_check_autocomplete env in
  ()
