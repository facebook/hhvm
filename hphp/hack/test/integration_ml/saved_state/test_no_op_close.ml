module Test = Integration_test_base
open Integration_test_base_types
open No_op_common

(* This test is the same as test_ide_open, except instead of a no-op OPEN, we start
 * with CLOSE. This happens to already not cause any rechecking, because hh_server just
 * crashes the request and closes persistent connection, but including it for completness
 * if in the future we change that behaviour. *)
let () = No_op_common.go @@ fun env ->
  let env = Test.connect_persistent_client env in
  (* We CLOSE the file without any changes. *)
  let env, _ = Test.close_file env foo_name ~ignore_response:true in
  let env, loop_output = Test.full_check env in
  Asserter.Int_asserter.assert_equals 1 loop_output.total_rechecked_count
    "Closing foo should not recheck all 10 of its dependencies";
  ignore env;
  ()
