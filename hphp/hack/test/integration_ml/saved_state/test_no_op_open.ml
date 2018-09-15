module Test = Integration_test_base
open Integration_test_base_types
open No_op_common

(* Tests whether no-op OPEN causes any crazy fan-outs *)
let () = No_op_common.go @@ fun env ->
  (* We open the file without any changes *)
  let env = Test.connect_persistent_client env in
  let env = Test.open_file env foo_name in
  let env, loop_output = Test.full_check env in
  Asserter.Int_asserter.assert_equals 2 loop_output.total_rechecked_count
    "Opening foo should not recheck all 10 of its dependencies";
  ignore env;
  ()
