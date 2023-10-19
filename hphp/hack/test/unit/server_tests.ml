(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

open Hh_prelude

let tcopt_with_defer =
  GlobalOptions.{ default with tco_defer_class_declaration_threshold = Some 1 }

let test_process_data =
  ServerProcess.
    {
      pid = 2758734;
      server_specific_files =
        { ServerCommandTypes.server_finale_file = "2758734.fin" };
      start_t = 0.0;
      in_fd = Unix.stdin;
      out_fds = [("default", Unix.stdout)];
      last_request_handoff = ref 0.0;
    }

let test_dmesg_parser () =
  let input =
    [
      "[3034339.262439] Out of memory: Kill process 2758734 (hh_server) score 253 or sacrifice child";
    ]
  in
  Sys_utils.For_test.find_oom_in_dmesg_output
    test_process_data.ServerProcess.pid
    "hh_server"
    input

let ensure_threshold ~(threshold : int) ~(decl_count : int) : unit =
  let result =
    Deferred_decl.with_deferred_decls
      ~enable:true
      ~declaration_threshold_opt:(Some threshold)
      ~memory_mb_threshold_opt:None
    @@ fun () ->
    for _i = 1 to decl_count do
      Deferred_decl.raise_if_should_defer ()
    done
  in
  match result with
  | Ok () ->
    Asserter.Bool_asserter.assert_equals
      (threshold > decl_count)
      true
      (Printf.sprintf
         "We've fetched %d decls, we should have reached the threshold %d"
         decl_count
         threshold)
  | Error () ->
    Asserter.Bool_asserter.assert_equals
      (threshold <= decl_count)
      true
      (Printf.sprintf
         "We've fetched %d decls, we should not have reached the threshold %d"
         decl_count
         threshold)

let test_deferred_decl_should_defer () =
  ensure_threshold ~threshold:0 ~decl_count:1;
  ensure_threshold ~threshold:1 ~decl_count:2;
  ensure_threshold ~threshold:2 ~decl_count:1;
  ensure_threshold ~threshold:1 ~decl_count:5;

  true

(* In this test, we wish to establish that we enable deferring type checking
   for files that have undeclared dependencies, UNLESS we've already deferred
   those files a certain number of times. *)
let test_process_file_deferring () =
  let { Common_setup.ctx; foo_path; _ } =
    Common_setup.setup ~sqlite:false tcopt_with_defer ~xhp_as:`Namespaces
  in
  let file =
    Typing_service_types.{ path = foo_path; was_already_deferred = false }
  in

  (* Finally, this is what all the setup was for: process this file *)
  Decl_counters.set_mode HackEventLogger.PerFileProfilingConfig.DeclingTopCounts;
  let prev_counter_state = Counters.reset () in
  let { Typing_check_service.deferred_decls; _ } =
    Typing_check_service.process_file
      ctx
      file
      ~log_errors:false
      ~decl_cap_mb:None
  in
  Counters.restore_state prev_counter_state;

  List.iter deferred_decls ~f:(fun (deferred_file, decl_name) ->
      Printf.printf "%s - %s\n" (Relative_path.suffix deferred_file) decl_name);
  (* Validate the declare file computation *)
  let found_declare =
    List.exists deferred_decls ~f:(fun (deferred_file, _) ->
        String.equal (Relative_path.suffix deferred_file) "Bar.php")
  in
  Asserter.Bool_asserter.assert_equals
    true
    found_declare
    "Should have found the declare file computation";
  let decl_names =
    List.fold deferred_decls ~init:SSet.empty ~f:(fun s (_, id) ->
        SSet.add id s)
  in
  let expected_decl_names =
    ["A"; "B"; "D"; "Foo"; "Bar"]
    |> List.map ~f:(Printf.sprintf "\\%s")
    |> SSet.of_list
  in
  Asserter.Bool_asserter.assert_equals
    true
    (SSet.equal decl_names expected_decl_names)
    (Printf.sprintf
       "Unexpected set of deferred decls. Expected %s, got %s"
       (SSet.show expected_decl_names)
       (SSet.show decl_names));
  Asserter.Int_asserter.assert_equals
    5
    (List.length deferred_decls)
    "Should be this many deferred_decls";

  true

let expected_decling_count = 71

(* This test verifies that the deferral/counting machinery works for
   ProviderUtils.compute_tast_and_errors_unquarantined. *)
let test_compute_tast_counting () =
  let { Common_setup.ctx; foo_path; foo_contents; _ } =
    Common_setup.setup ~sqlite:false tcopt_with_defer ~xhp_as:`Namespaces
  in

  let (ctx, entry) =
    Provider_context.add_or_overwrite_entry_contents
      ~ctx
      ~path:foo_path
      ~contents:foo_contents
  in
  let { Tast_provider.Compute_tast_and_errors.telemetry; _ } =
    Tast_provider.compute_tast_and_errors_unquarantined ~ctx ~entry
  in

  Asserter.Int_asserter.assert_equals
    expected_decling_count
    (Telemetry_test_utils.int_exn telemetry "decling.count")
    "There should be this many decling_count for shared_mem provider";

  (* We'll read Bar.php from disk when we decl-parse it in order to compute the
     TAST of Foo.php, but we won't read Foo.php from disk to get its AST or
     decls, since we want to use the contents in the Provider_context entry. *)
  Asserter.Int_asserter.assert_equals
    1
    (Telemetry_test_utils.int_exn telemetry "disk_cat.count")
    "There should be 1 disk_cat_count for shared_mem provider";

  true

(* This test verifies that the deferral/counting machinery works for
   ProviderUtils.compute_tast_and_errors_unquarantined, this time with local memory backend. *)
let test_compute_tast_counting_local_mem () =
  Utils.with_context
    ~enter:(fun () ->
      Provider_backend.set_local_memory_backend_with_defaults_for_test ())
    ~exit:(fun () ->
      (* restore it back to shared_mem for the rest of the tests *)
      Provider_backend.set_shared_memory_backend ())
    ~do_:(fun () ->
      let { Common_setup.ctx; foo_path; foo_contents; _ } =
        Common_setup.setup ~sqlite:false tcopt_with_defer ~xhp_as:`Namespaces
      in
      let (ctx, entry) =
        Provider_context.add_or_overwrite_entry_contents
          ~ctx
          ~path:foo_path
          ~contents:foo_contents
      in
      let { Tast_provider.Compute_tast_and_errors.telemetry; _ } =
        Tast_provider.compute_tast_and_errors_unquarantined ~ctx ~entry
      in
      Asserter.Int_asserter.assert_equals
        expected_decling_count
        (Telemetry_test_utils.int_exn telemetry "decling.count")
        "There should be this many decling_count for local_memory provider";
      Asserter.Int_asserter.assert_equals
        1
        (Telemetry_test_utils.int_exn telemetry "disk_cat.count")
        "There should be 1 disk_cat_count for local_memory_provider");

  true

let test_should_enable_deferring () =
  Relative_path.set_path_prefix
    Relative_path.Root
    (Path.make @@ Common_setup.in_fake_dir "www");

  let file =
    Typing_service_types.
      {
        path =
          Relative_path.create Relative_path.Root
          @@ Common_setup.in_fake_dir "www/Foo.php";
        was_already_deferred = false;
      }
  in
  Asserter.Bool_asserter.assert_equals
    true
    (Typing_check_service.should_enable_deferring file)
    "File should be deferred twice - below max";

  let file = Typing_service_types.{ file with was_already_deferred = true } in
  Asserter.Bool_asserter.assert_equals
    false
    (Typing_check_service.should_enable_deferring file)
    "File should not be deferred - at max";

  let file = Typing_service_types.{ file with was_already_deferred = true } in
  Asserter.Bool_asserter.assert_equals
    false
    (Typing_check_service.should_enable_deferring file)
    "File should not be deferred - above max";

  true

(* This test verifies quarantine. *)
let test_quarantine () =
  Provider_backend.set_local_memory_backend_with_defaults_for_test ();
  let { Common_setup.ctx; foo_path; foo_contents; nonexistent_path; _ } =
    Common_setup.setup ~sqlite:false tcopt_with_defer ~xhp_as:`Namespaces
  in

  (* simple case *)
  let (ctx, _foo_entry) =
    Provider_context.add_or_overwrite_entry_contents
      ~ctx
      ~path:foo_path
      ~contents:foo_contents
  in
  let can_quarantine =
    try
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          "ok")
    with
    | e -> e |> Exception.wrap |> Exception.to_string
  in
  Asserter.String_asserter.assert_equals
    "ok"
    can_quarantine
    "Should be able to quarantine foo";

  (* repeat of simple case *)
  let can_quarantine =
    try
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          "ok")
    with
    | e -> e |> Exception.wrap |> Exception.to_string
  in
  Asserter.String_asserter.assert_equals
    "ok"
    can_quarantine
    "Should be able to quarantine foo a second time";

  (* add a non-existent file; should fail *)
  let (ctx2, _nonexistent_entry) =
    Provider_context.add_or_overwrite_entry_contents
      ~ctx
      ~path:nonexistent_path
      ~contents:""
  in
  let can_quarantine =
    try
      Provider_utils.respect_but_quarantine_unsaved_changes
        ~ctx:ctx2
        ~f:(fun () -> "ok")
    with
    | e -> e |> Exception.wrap |> Exception.to_string
  in
  Asserter.String_asserter.assert_equals
    "ok"
    can_quarantine
    "Should be able to quarantine nonexistent_file";

  (* repeat of simple case, back with original ctx *)
  let can_quarantine =
    try
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          "ok")
    with
    | e -> e |> Exception.wrap |> Exception.to_string
  in
  Asserter.String_asserter.assert_equals
    "ok"
    can_quarantine
    "Should be able to quarantine foo a third time";

  true

let tests =
  [
    ("test_deferred_decl_should_defer", test_deferred_decl_should_defer);
    ("test_process_file_deferring", test_process_file_deferring);
    ("test_compute_tast_counting", test_compute_tast_counting);
    ( "test_compute_tast_counting_local_mem",
      test_compute_tast_counting_local_mem );
    ("test_dmesg_parser", test_dmesg_parser);
    ("test_should_enable_deferring", test_should_enable_deferring);
    ("test_quarantine", test_quarantine);
  ]

let () =
  EventLogger.init_fake ();
  (* The parsing service needs shared memory to be set up *)
  let config =
    SharedMem.
      {
        global_size = 1024;
        heap_size = 1024 * 8;
        hash_table_pow = 10;
        shm_dirs = [];
        shm_use_sharded_hashtbl = false;
        shm_cache_size = -1;
        shm_min_avail = 0;
        log_level = 0;
        sample_rate = 0.0;
        compression = 0;
      }
  in
  let (_ : SharedMem.handle) = SharedMem.init config ~num_workers:0 in
  tests
  |> List.map ~f:(fun (name, do_) ->
         ( name,
           fun () ->
             Utils.with_context
               ~enter:Provider_backend.set_shared_memory_backend
               ~exit:(fun () -> ())
               ~do_ ))
  |> Unit_test.run_all
