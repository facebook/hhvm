module Test = Integration_test_base

let foo_name = "foo.php"

let bar_name = Printf.sprintf "bar%d.php"

let foo_contents =
  "<?hh //strict
/* HH_FIXME[4336] */
function foo() : string {
}
"

let bar_contents =
  Printf.sprintf "<?hh //strict

function bar%d() : int {
  return foo();
}
"

let expected_errors =
  {|
File "/bar2.php", line 4, characters 10-14:
Invalid return type (Typing[4110])
File "/bar2.php", line 3, characters 19-21:
Expected int
File "/foo.php", line 3, characters 18-23:
But got string
|}

let test () =
  let env = Test.setup_server () in
  (* There are errors in both bar files *)
  let env =
    Test.setup_disk
      env
      [
        (foo_name, foo_contents);
        (bar_name 1, bar_contents 1);
        (bar_name 2, bar_contents 2);
      ]
  in
  (* Prepare rechecking of all files *)
  let ctx = Provider_utils.ctx_from_server_env env in
  let workers = None in
  let fast = Naming_table.to_fast env.ServerEnv.naming_table in
  (* Pretend that this rechecking will be cancelled before we get to bar1 *)
  let bar1_path =
    Relative_path.(create Root (Test.prepend_root (bar_name 1)))
  in
  Typing_check_service.TestMocking.set_is_cancelled bar1_path;

  (* Run the recheck *)
  let interrupt = MultiThreadedCall.no_interrupt () in
  let fnl = Relative_path.Map.keys fast in
  let check_info =
    {
      Typing_check_service.init_id = "";
      recheck_id = Some "";
      profile_log = false;
      profile_type_check_twice = false;
      profile_type_check_duration_threshold = 0.;
    }
  in
  let (errors, _delegate_state, _telemetry, (), cancelled) =
    Typing_check_service.go_with_interrupt
      ctx
      workers
      (Typing_service_delegate.create ())
      (Telemetry.create ())
      Relative_path.Set.empty
      fnl
      ~interrupt
      ~memory_cap:None
      ~check_info
  in
  (* Assert that we got the errors in bar2 only... *)
  Test.assert_errors errors expected_errors;

  (* ...while bar1 is among cancelled jobs*)
  (match cancelled with
  | [x] when x = bar1_path -> ()
  | _ -> assert false);
  ()
