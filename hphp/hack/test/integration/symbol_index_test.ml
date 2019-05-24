module Args = Test_harness_common_args

let run_hh_check
    (hh_client_path: string)
    (repo_path: string): unit =
    let hh_client_process = Process.exec
        hh_client_path
        ["check"; repo_path]
    in
    let open Process_types in
    match Process.read_and_wait_pid ~timeout:75
      hh_client_process with
    | Error Abnormal_exit { stdout; _} ->
      let errmsg = Printf.sprintf "Failed to run hh_client: %s" stdout in
      failwith errmsg;
    | Error Timed_out _ ->
      failwith "Timed out trying to run hh check";
    | Error _ ->
      failwith "Failed: other error";
    | Ok _ ->
      ()
;;

let run_autocomplete
    (hh_client_path: string)
    (repo_path: string)
    (context: string): string =
  let hh_client_process = Process.exec
      hh_client_path ~input:context
      [repo_path; "--auto-complete"]
  in
  let open Process_types in
  let results = match Process.read_and_wait_pid ~timeout:75
    hh_client_process with
  | Error Abnormal_exit { stdout; _} ->
    let errmsg = Printf.sprintf "Failed to run hh_client: %s" stdout in
    failwith errmsg;
  | Error Timed_out _ ->
    failwith "Timed out trying to get autocomplete";
  | Error _ ->
    failwith "Failed: other error";
  | Ok {stdout; _} ->
    stdout
  in
  results
;;

let test_basic_results (harness: Test_harness.t): bool =
  let open Test_harness in
  let hh_client_path = harness.hh_client_path in
  let hh_server_path = harness.hh_client_path in
  let repo_path = Path.to_string harness.repo_dir in

  (* Launch hh for a one time check *)
  run_hh_check hh_client_path repo_path;
  Printf.printf "HH Check finished [%s]\n%!" repo_path;

  (* Request autocomplete of type "Acnew" *)
  let context_string = "<?hh function f() { $x = new UsesAUTO332" in
  let results = run_autocomplete hh_client_path repo_path context_string in
  Asserter.String_asserter.assert_equals "UsesA class\n" results
    "Should be able to find autocomplete for 'UsesA'";

  (* Request autocomplete of type "Actrait_only" *)
  let context_string = "<?hh final class EntFoo extends Ent { use NoBigTrAUTO332" in
  let results = run_autocomplete hh_client_path repo_path context_string in
  Asserter.String_asserter.assert_equals "NoBigTrait trait\n" results
    "Should be able to find autocomplete for 'NoBigTrait'";

  (* Request autocomplete of type "Acid" *)
  let context_string = "<?hh function f() { some_long_AUTO332" in
  let results = run_autocomplete hh_client_path repo_path context_string in
  Asserter.String_asserter.assert_equals "some_long_function_name _\n" results
    "Should be able to find autocomplete for 'some_long_function_name'";

  (* Request autocomplete of type "Actype" *)
  let context_string = "<?hh function f(ClassToBeAUTO332" in
  let results = run_autocomplete hh_client_path repo_path context_string in
  Asserter.String_asserter.assert_equals "ClassToBeIdentified class\n" results
    "Should be able to find autocomplete for 'ClassToBeIdentified'";

  (* If we got here, all is well *)
  true

;;

let tests args =
  let harness_config = {
    Test_harness.hh_server = args.Args.hh_server;
    hh_client = args.Args.hh_client;
    template_repo = args.Args.template_repo;
  } in
  [
  ("test_basic_results", fun () ->
    Test_harness.run_test ~stop_server_in_teardown:false harness_config
    test_basic_results);
  ]
;;

let () =
  Daemon.check_entry_point ();
  let args = Args.parse () in
  let () = HackEventLogger.client_init (args.Args.template_repo) in
  Unit_test.run_all (tests args)
