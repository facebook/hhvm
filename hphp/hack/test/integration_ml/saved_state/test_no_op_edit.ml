module Test = Integration_test_base
open Integration_test_base_types
open No_op_common

(* This test is the same as test_ide_open, except instead of no-op OPEN, we start
 * with no-op EDIT. Nuclide does send didEdit without preceding didOpen sometimes (see T30353394) *)
let () = No_op_common.go @@ fun env ->
  let env = Test.connect_persistent_client env in
  (* We EDIT the file without any changes *)
  let env, _ = Test.edit_file env foo_name foo_contents in
  let env, loop_output = Test.full_check env in
  Asserter.Int_asserter.assert_equals 2 loop_output.total_rechecked_count
    "Editing foo should not recheck all 10 of its dependencies";
  ignore env;
  ()
