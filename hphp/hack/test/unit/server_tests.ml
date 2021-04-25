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
        {
          ServerCommandTypes.server_finale_file = "2758734.fin";
          server_progress_file = "progress.2758734.json";
        };
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

let ensure_count (count : int) : unit =
  let deferred = Deferred_decl.get_deferments () in
  Asserter.Int_asserter.assert_equals
    count
    (List.length deferred)
    "The number of deferred items should match the expected value"

let test_deferred_decl_add () =
  Deferred_decl.reset
    ~enable:true
    ~declaration_threshold_opt:None
    ~memory_mb_threshold_opt:None;
  ensure_count 0;

  Deferred_decl.add_deferment
    ~d:(Relative_path.create Relative_path.Dummy "foo", "\\Foo");
  ensure_count 1;

  Deferred_decl.add_deferment
    ~d:(Relative_path.create Relative_path.Dummy "foo", "\\Foo");
  ensure_count 1;

  Deferred_decl.add_deferment
    ~d:(Relative_path.create Relative_path.Dummy "bar", "\\Bar");
  ensure_count 2;

  Deferred_decl.reset
    ~enable:true
    ~declaration_threshold_opt:None
    ~memory_mb_threshold_opt:None;
  ensure_count 0;

  true

let ensure_threshold ~(threshold : int) ~(limit : int) ~(expected : int) : unit
    =
  Deferred_decl.reset
    ~enable:true
    ~declaration_threshold_opt:(Some threshold)
    ~memory_mb_threshold_opt:None;
  ensure_count 0;

  let deferred_count = ref 0 in
  for i = 1 to limit do
    let path = Printf.sprintf "foo-%d" i in
    let relative_path = Relative_path.create Relative_path.Dummy path in
    try
      Deferred_decl.raise_if_should_defer ~deferment:(relative_path, "\\Foo");
      Deferred_decl.increment_counter ()
    with Deferred_decl.Defer (d, _) ->
      Asserter.Bool_asserter.assert_equals
        (i >= threshold)
        true
        (Printf.sprintf
           "We should have reached the threshold %d, i=%d"
           threshold
           i);
      Asserter.String_asserter.assert_equals
        (Relative_path.suffix d)
        path
        "The deferred path should be the last one we saw";
      deferred_count := !deferred_count + 1
  done;

  Asserter.Int_asserter.assert_equals
    expected
    !deferred_count
    (Printf.sprintf
       "Unexpected deferred count; threshold: %d; limit: %d; expected: %d"
       threshold
       limit
       expected)

let test_deferred_decl_should_defer () =
  ensure_threshold ~threshold:0 ~limit:1 ~expected:1;
  ensure_threshold ~threshold:1 ~limit:2 ~expected:1;
  ensure_threshold ~threshold:2 ~limit:1 ~expected:0;
  ensure_threshold ~threshold:1 ~limit:5 ~expected:4;

  true

(* In this test, we wish to establish that we enable deferring type checking
  for files that have undeclared dependencies, UNLESS we've already deferred
  those files a certain number of times. *)
let test_process_file_deferring () =
  let { Common_setup.ctx; foo_path; _ } =
    Common_setup.setup ~sqlite:false tcopt_with_defer
  in
  let file = Typing_service_types.{ path = foo_path; deferred_count = 0 } in
  let dynamic_view_files = Relative_path.Set.empty in
  let errors = Errors.empty in

  (* Finally, this is what all the setup was for: process this file *)
  Decl_counters.set_mode Typing_service_types.DeclingTopCounts;
  let prev_counter_state = Counters.reset () in
  let { Typing_check_service.deferred_decls; _ } =
    Typing_check_service.process_file dynamic_view_files ctx errors file
  in
  let counters = Counters.get_counters () in
  Counters.restore_state prev_counter_state;
  Asserter.Int_asserter.assert_equals
    1
    (List.length deferred_decls)
    "Should be this many deferred_decls";

  (* this test doesn't write back to cache, so num of decl_fetches isn't solid *)
  Asserter.Bool_asserter.assert_equals
    true
    (Telemetry_test_utils.int_exn counters "decling.count" > 0)
    "Should be at least one decl fetched";

  (* Validate the declare file computation *)
  let found_declare =
    List.exists deferred_decls ~f:(fun (deferred_file, _) ->
        Asserter.String_asserter.assert_equals
          "Bar.php"
          (Relative_path.suffix deferred_file)
          "Declare path must match the expected";
        true)
  in
  Asserter.Bool_asserter.assert_equals
    true
    found_declare
    "Should have found the declare file computation";

  true

(* This test verifies that the deferral/counting machinery works for
   ProviderUtils.compute_tast_and_errors_unquarantined. *)
let test_compute_tast_counting () =
  let { Common_setup.ctx; foo_path; foo_contents; _ } =
    Common_setup.setup ~sqlite:false tcopt_with_defer
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
    40
    (Telemetry_test_utils.int_exn telemetry "decling.count")
    "There should be this many decling_count for shared_mem provider";
  Asserter.Int_asserter.assert_equals
    0
    (Telemetry_test_utils.int_exn telemetry "disk_cat.count")
    "There should be 0 disk_cat_count for shared_mem provider";

  (* Now try the same with local_memory backend *)
  Utils.with_context
    ~enter:(fun () ->
      Provider_backend.set_local_memory_backend_with_defaults ())
    ~exit:(fun () ->
      (* restore it back to shared_mem for the rest of the tests *)
      Provider_backend.set_shared_memory_backend ())
    ~do_:(fun () ->
      let ctx =
        Provider_context.empty_for_tool
          ~popt:ParserOptions.default
          ~tcopt:TypecheckerOptions.default
          ~backend:(Provider_backend.get ())
          ~deps_mode:Typing_deps_mode.SQLiteMode
      in
      let (ctx, entry) =
        Provider_context.add_entry_if_missing ~ctx ~path:foo_path
      in
      let { Tast_provider.Compute_tast_and_errors.telemetry; _ } =
        Tast_provider.compute_tast_and_errors_unquarantined ~ctx ~entry
      in
      Asserter.Int_asserter.assert_equals
        40
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

  let opts =
    GlobalOptions.{ default with tco_max_times_to_defer_type_checking = Some 2 }
  in
  let file =
    Typing_service_types.
      {
        path =
          Relative_path.create Relative_path.Root
          @@ Common_setup.in_fake_dir "www/Foo.php";
        deferred_count = 1;
      }
  in
  Asserter.Bool_asserter.assert_equals
    true
    (Typing_check_service.should_enable_deferring opts file)
    "File should be deferred twice - below max";

  let file = Typing_service_types.{ file with deferred_count = 2 } in
  Asserter.Bool_asserter.assert_equals
    false
    (Typing_check_service.should_enable_deferring opts file)
    "File should not be deferred - at max";

  let file = Typing_service_types.{ file with deferred_count = 3 } in
  Asserter.Bool_asserter.assert_equals
    false
    (Typing_check_service.should_enable_deferring opts file)
    "File should not be deferred - above max";

  let opts =
    GlobalOptions.{ default with tco_max_times_to_defer_type_checking = None }
  in
  Asserter.Bool_asserter.assert_equals
    true
    (Typing_check_service.should_enable_deferring opts file)
    "File should be deferred - max is not set";

  true

(* This test verifies quarantine. *)
let test_quarantine () =
  Provider_backend.set_local_memory_backend_with_defaults ();
  let { Common_setup.ctx; foo_path; foo_contents; nonexistent_path; _ } =
    Common_setup.setup ~sqlite:false tcopt_with_defer
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
    with e -> e |> Exception.wrap |> Exception.to_string
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
    with e -> e |> Exception.wrap |> Exception.to_string
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
    with e -> e |> Exception.wrap |> Exception.to_string
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
    with e -> e |> Exception.wrap |> Exception.to_string
  in
  Asserter.String_asserter.assert_equals
    "ok"
    can_quarantine
    "Should be able to quarantine foo a third time";

  true

let tests =
  [
    ("test_deferred_decl_add", test_deferred_decl_add);
    ("test_deferred_decl_should_defer", test_deferred_decl_should_defer);
    ("test_process_file_deferring", test_process_file_deferring);
    ("test_compute_tast_counting", test_compute_tast_counting);
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
        dep_table_pow = 16;
        hash_table_pow = 10;
        shm_dirs = [];
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
