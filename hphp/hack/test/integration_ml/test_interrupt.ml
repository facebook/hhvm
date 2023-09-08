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
  Expected `int`
  File "/foo.php", line 3, characters 18-23:
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
  let options = ServerArgs.default_options ~root in
  let (custom_config, _) = ServerConfig.load ~silent:false options in
  let env = Test.setup_server ~custom_config () in
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
  let defs_per_file =
    Naming_table.to_defs_per_file env.ServerEnv.naming_table
  in
  (* Pretend that this rechecking will be cancelled before we get to bar1 *)
  let bar1_path =
    Relative_path.(create Root (Test.prepend_root (bar_name 1)))
  in
  Typing_check_service.TestMocking.set_is_cancelled bar1_path;

  (* Run the recheck *)
  let interrupt = MultiThreadedCall.no_interrupt () in
  let fnl = Relative_path.Map.keys defs_per_file in
  let check_info =
    {
      Typing_service_types.init_id = "";
      check_reason = "test_interrupt";
      log_errors = false;
      recheck_id = Some "";
      use_max_typechecker_worker_memory_for_decl_deferral = false;
      per_file_profiling = HackEventLogger.PerFileProfilingConfig.default;
      memtrace_dir = None;
    }
  in
  let (((), { Typing_check_service.errors; _ }), cancelled) =
    Typing_check_service.go_with_interrupt
      ctx
      workers
      (Telemetry.create ())
      fnl
      ~root:None
      ~interrupt
      ~memory_cap:None
      ~longlived_workers:false
      ~use_hh_distc_instead_of_hulk:false
      ~hh_distc_fanout_threshold:None
      ~check_info
  in
  (* Assert that we got the errors in bar2 only... *)
  Test.assert_errors errors expected_errors;

  (* ...while bar1 is among cancelled jobs*)
  (match cancelled with
  | Some ([x], _) when x = bar1_path -> ()
  | _ -> assert false);
  ()
