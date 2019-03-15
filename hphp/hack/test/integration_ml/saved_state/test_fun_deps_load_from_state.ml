(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

open Hh_core

module Test = Integration_test_base

let file = "test.php", {|<?hh // strict
class Test {
  public function baz(int $i, (function(): void) $f): void {
    $f();
  }
}
|}

let () =
  Tempfile.with_real_tempdir @@ fun temp_dir ->
    let temp_dir = Path.to_string temp_dir in
    let disk_state = [file] in
    Test.save_state disk_state temp_dir;
    let env =
      Test.load_state
        temp_dir
        ~disk_state
        ~master_changes:[]
        ~local_changes:[]
        ~use_precheked_files:false
      in
    let ServerEnv.{tcopt; naming_table; _} = env in
    let _ =
      let filter = ServerCommandTypes.Method_jumps.No_filter in
      let find_children = true in
      let class_ = "Test" in
      MethodJumps.get_inheritance class_ ~filter ~find_children
        env.ServerEnv.naming_table ServerEnvBuild.default_genv.ServerEnv.workers in
    let expected = {|
      {"position":{"file":"/test.php","line":3,"character":19},
       "deps":[{"name":"$f","kind":"local",
       "position":{"filename":"/test.php","line":3,"char_start":50,"char_end":51}}]} |}
     |> String.split_on_char '\n'
     |> List.map ~f:String.trim
     |> String.concat "" in

    let h = ServerFunDepsBatch.handlers in
    let pos_infos, errors =
      ServerRxApiShared.prepare_pos_infos h [("/" ^ (fst file), 3, 19)] naming_table in
    if errors <> []
    then Test.fail ("Unexpected errors:" ^ (String.concat "," errors));
    let result = ServerRxApiShared.helper h tcopt [] pos_infos in

    if result <> [expected]
    then begin
      let msg =
        "Unexpected test result\nExpected:\n" ^
        expected ^ "\nBut got:\n" ^
        (String.concat "\n" result) in
      Test.fail msg
    end
