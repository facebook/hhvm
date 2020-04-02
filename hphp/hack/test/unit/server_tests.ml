(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

open Core_kernel

let errors_to_string (errors : Errors.t) : string list =
  let error_to_string (error : Errors.error) : string =
    let error = Errors.to_absolute_for_test error in
    let code = Errors.get_code error in
    let message =
      error |> Errors.to_list |> List.map ~f:snd |> String.concat ~sep:"; "
    in
    Printf.sprintf "[%d] %s" code message
  in
  errors |> Errors.get_sorted_error_list |> List.map ~f:error_to_string

let fake_dir = Printf.sprintf "%s/fake" (Filename.get_temp_dir_name ())

let in_fake_dir path = Printf.sprintf "%s/%s" fake_dir path

let test_process_data =
  ServerProcess.
    {
      pid = 2758734;
      finale_file = "2758734.fin";
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
  Sys_utils.find_oom_in_dmesg_output
    test_process_data.ServerProcess.pid
    "hh_server"
    input

let ensure_count (count : int) : unit =
  let deferred = Deferred_decl.get_deferments ~f:(fun d -> d) in
  Asserter.Int_asserter.assert_equals
    count
    (List.length deferred)
    "The number of deferred items should match the expected value"

let test_deferred_decl_add () =
  Deferred_decl.reset ~enable:true ~threshold_opt:None;
  ensure_count 0;

  Deferred_decl.add_deferment
    ~d:(Relative_path.create Relative_path.Dummy "foo");
  ensure_count 1;

  Deferred_decl.add_deferment
    ~d:(Relative_path.create Relative_path.Dummy "foo");
  ensure_count 1;

  Deferred_decl.add_deferment
    ~d:(Relative_path.create Relative_path.Dummy "bar");
  ensure_count 2;

  Deferred_decl.reset ~enable:true ~threshold_opt:None;
  ensure_count 0;

  true

let ensure_threshold ~(threshold : int) ~(limit : int) ~(expected : int) : unit
    =
  Deferred_decl.reset ~enable:true ~threshold_opt:(Some threshold);
  ensure_count 0;

  let deferred_count = ref 0 in
  for i = 1 to limit do
    let path = Printf.sprintf "foo-%d" i in
    let relative_path = Relative_path.create Relative_path.Dummy path in
    try
      Deferred_decl.raise_if_should_defer ~d:relative_path;
      Deferred_decl.increment_counter ()
    with Deferred_decl.Defer d ->
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

let foo_contents =
  {|<?hh //strict
  class Foo {
    public function foo (Bar $b) : int {
      return $b->toString();
    }
  }

  function f1(Foo $x) : void { }
  function f2(foo $x) : void { }
|}

let bar_contents =
  {|<?hh //strict
  class Bar {
    public function toString() : string {
      return "bar";
    }
  }
|}

type server_setup = {
  ctx: Provider_context.t;
  foo_path: Relative_path.t;
  foo_contents: string;
  bar_path: Relative_path.t;
  bar_contents: string;
  nonexistent_path: Relative_path.t;
  naming_table: Naming_table.t;
}

(** This lays down some files on disk. Sets up a forward naming table.
Sets up the provider's reverse naming table. Returns an empty context, plus
information about the files on disk. *)
let server_setup () : server_setup =
  (* Set up a simple fake repo *)
  Disk.mkdir_p @@ in_fake_dir "root/";
  Relative_path.set_path_prefix
    Relative_path.Root
    (Path.make @@ in_fake_dir "root/");

  (* We'll need to parse these files in order to create a naming table, which
    will be used for look up of symbols in type checking. *)
  Disk.write_file ~file:(in_fake_dir "root/Foo.php") ~contents:foo_contents;
  Disk.write_file ~file:(in_fake_dir "root/Bar.php") ~contents:bar_contents;
  let foo_path = Relative_path.from_root "Foo.php" in
  let bar_path = Relative_path.from_root "Bar.php" in
  let nonexistent_path = Relative_path.from_root "Nonexistent.php" in
  (* Parsing produces the file infos that the naming table module can use
    to construct the forward naming table (files-to-symbols) *)
  let popt = ParserOptions.default in
  let (file_infos, _errors, _failed_parsing) =
    Parsing_service.go
      None
      Relative_path.Set.empty
      ~get_next:(MultiWorker.next None [foo_path; bar_path])
      popt
      ~trace:true
  in
  let naming_table = Naming_table.create file_infos in
  (* Construct the reverse naming table (symbols-to-files) *)
  let fast = Naming_table.to_fast naming_table in
  let tcopt =
    GlobalOptions.
      { default with tco_defer_class_declaration_threshold = Some 1 }
  in
  let ctx =
    Provider_context.empty_for_tool
      ~popt
      ~tcopt
      ~backend:(Provider_backend.get ())
  in
  Relative_path.Map.iter fast ~f:(fun name info ->
      let {
        FileInfo.n_classes = classes;
        n_record_defs = record_defs;
        n_types = typedefs;
        n_funs = funs;
        n_consts = consts;
      } =
        info
      in
      Naming_global.ndecl_file_fast
        ctx
        name
        ~funs
        ~classes
        ~record_defs
        ~typedefs
        ~consts);

  (* Construct one instance of file type check computation. Class \Foo depends
    on class \Bar being declared, so processing \Foo once should result in
    two computations:
      - a declaration of \Bar
      - a (deferred) type check of \Foo *)
  {
    ctx;
    foo_path;
    foo_contents;
    bar_path;
    bar_contents;
    nonexistent_path;
    naming_table;
  }

(* In this test, we wish to establish that we enable deferring type checking
  for files that have undeclared dependencies, UNLESS we've already deferred
  those files a certain number of times. *)
let test_process_file_deferring () =
  let { ctx; foo_path; _ } = server_setup () in
  let file = Typing_check_service.{ path = foo_path; deferred_count = 0 } in
  let dynamic_view_files = Relative_path.Set.empty in
  let errors = Errors.empty in

  (* Finally, this is what all the setup was for: process this file *)
  let { Typing_check_service.computation; counters; _ } =
    Typing_check_service.process_file
      dynamic_view_files
      ctx
      ~profile_log:true
      errors
      file
  in
  Asserter.Int_asserter.assert_equals
    2
    (List.length computation)
    "Should be two file computations";

  (* this test doesn't write back to cache, so num of decl_fetches isn't solid *)
  Asserter.Bool_asserter.assert_equals
    true
    (Telemetry_test_utils.int_exn counters "decl_accessors.count" > 0)
    "Should be at least one decl fetched";

  (* Validate the deferred type check computation *)
  let found_check =
    List.exists computation ~f:(fun file_computation ->
        match file_computation with
        | Typing_check_service.(Check { path; deferred_count }) ->
          Asserter.String_asserter.assert_equals
            "Foo.php"
            (Relative_path.suffix path)
            "Check path must match the expected";
          Asserter.Int_asserter.assert_equals
            1
            deferred_count
            "Check deferred count must match the expected";
          true
        | _ -> false)
  in
  Asserter.Bool_asserter.assert_equals
    true
    found_check
    "Should have found the check file computation";

  (* Validate the declare file computation *)
  let found_declare =
    List.exists computation ~f:(fun file_computation ->
        match file_computation with
        | Typing_check_service.Declare path ->
          Asserter.String_asserter.assert_equals
            "Bar.php"
            (Relative_path.suffix path)
            "Declare path must match the expected";
          true
        | _ -> false)
  in
  Asserter.Bool_asserter.assert_equals
    true
    found_declare
    "Should have found the declare file computation";

  true

(* This test verifies that the deferral/counting machinery works for
   ProviderUtils.compute_tast_and_errors_unquarantined. *)
let test_compute_tast_counting () =
  let { ctx; foo_path; foo_contents; _ } = server_setup () in

  let (ctx, entry) =
    Provider_context.add_entry_from_file_contents
      ~ctx
      ~path:foo_path
      ~contents:foo_contents
  in
  let { Tast_provider.Compute_tast_and_errors.telemetry; _ } =
    Tast_provider.compute_tast_and_errors_unquarantined ~ctx ~entry
  in

  Asserter.Int_asserter.assert_equals
    125
    (Telemetry_test_utils.int_exn telemetry "decl_accessors.count")
    "There should be this many decl_accessor_count for shared_mem provider";
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
      in
      let (ctx, entry) = Provider_context.add_entry ~ctx ~path:foo_path in
      let { Tast_provider.Compute_tast_and_errors.telemetry; _ } =
        Tast_provider.compute_tast_and_errors_unquarantined ~ctx ~entry
      in
      Asserter.Int_asserter.assert_equals
        63
        (Telemetry_test_utils.int_exn telemetry "decl_accessors.count")
        "There should be this many decl_accessor_count for local_memory provider";
      Asserter.Int_asserter.assert_equals
        0
        (Telemetry_test_utils.int_exn telemetry "disk_cat.count")
        "There should be 0 disk_cat_count for local_memory_provider");

  true

let test_should_enable_deferring () =
  Relative_path.set_path_prefix
    Relative_path.Root
    (Path.make @@ in_fake_dir "www");

  let opts =
    GlobalOptions.{ default with tco_max_times_to_defer_type_checking = Some 2 }
  in
  let file =
    Typing_check_service.
      {
        path =
          Relative_path.create Relative_path.Root @@ in_fake_dir "www/Foo.php";
        deferred_count = 1;
      }
  in
  Asserter.Bool_asserter.assert_equals
    true
    (Typing_check_service.should_enable_deferring opts file)
    "File should be deferred twice - below max";

  let file = Typing_check_service.{ file with deferred_count = 2 } in
  Asserter.Bool_asserter.assert_equals
    false
    (Typing_check_service.should_enable_deferring opts file)
    "File should not be deferred - at max";

  let file = Typing_check_service.{ file with deferred_count = 3 } in
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
  let { ctx; foo_path; foo_contents; nonexistent_path; _ } = server_setup () in

  (* simple case *)
  let (ctx, _foo_entry) =
    Provider_context.add_entry_from_file_contents
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
    Provider_context.add_entry_from_file_contents
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

let test_unsaved_symbol_change () =
  Provider_backend.set_local_memory_backend_with_defaults ();

  (* We'll create a naming-table. This test suite is based on naming-table. *)
  let { ctx; foo_path; foo_contents; naming_table; _ } = server_setup () in
  let db_name = Filename.temp_file "server_naminng" ".sqlite" in
  let save_results = Naming_table.save naming_table db_name in
  Asserter.Int_asserter.assert_equals
    2
    save_results.Naming_sqlite.files_added
    "unsaved: expected this many files in naming.sqlite";
  Asserter.Int_asserter.assert_equals
    4
    save_results.Naming_sqlite.symbols_added
    "unsaved: expected this many symbols in naming.sqlite";

  (* Now, I want a fresh ctx with no reverse-naming entries in it,
  and I want it to be backed by a sqlite naming database. *)
  let (_ : Naming_table.t) = Naming_table.load_from_sqlite ctx db_name in
  Provider_backend.set_local_memory_backend_with_defaults ();
  let ctx =
    Provider_context.empty_for_tool
      ~popt:(Provider_context.get_popt ctx)
      ~tcopt:(Provider_context.get_tcopt ctx)
      ~backend:(Provider_backend.get ())
  in

  (* Compute tast as-is *)
  let (ctx, entry) =
    Provider_context.add_entry_from_file_contents
      ~ctx
      ~path:foo_path
      ~contents:foo_contents
  in
  let { Tast_provider.Compute_tast_and_errors.telemetry; errors; _ } =
    Tast_provider.compute_tast_and_errors_unquarantined ~ctx ~entry
  in
  Asserter.Int_asserter.assert_equals
    8
    (Telemetry_test_utils.int_exn telemetry "get_ast.count")
    "unsaved: compute_tast(class Foo) should have this many calls to get_ast";
  Asserter.Int_asserter.assert_equals
    1
    (Telemetry_test_utils.int_exn telemetry "disk_cat.count")
    "unsaved: compute_tast(class Foo) should have this many calls to disk_cat";
  Asserter.String_asserter.assert_list_equals
    [
      "[4110] Invalid return type; Expected int; But got string";
      "[2006] Could not find foo; Did you mean Foo?";
    ]
    (errors_to_string errors)
    "unsaved: compute_tast(class Foo) should have these errors";

  (* Make an unsaved change which affects a symbol definition that's used *)
  let foo_contents1 =
    Str.global_replace (Str.regexp "class Foo") "class Foo1" foo_contents
  in
  let (ctx, entry) =
    Provider_context.add_entry_from_file_contents
      ~ctx
      ~path:foo_path
      ~contents:foo_contents1
  in
  let { Tast_provider.Compute_tast_and_errors.telemetry; errors; _ } =
    Tast_provider.compute_tast_and_errors_unquarantined ~ctx ~entry
  in
  Asserter.Int_asserter.assert_equals
    1
    (Telemetry_test_utils.int_exn telemetry "get_ast.count")
    "unsaved: compute_tast(class Foo1) should have this many calls to get_ast";
  Asserter.Int_asserter.assert_equals
    0
    (Telemetry_test_utils.int_exn telemetry "disk_cat.count")
    "unsaved: compute_tast(class Foo1) should have this many calls to disk_cat";
  Asserter.String_asserter.assert_list_equals
    [
      "[4110] Invalid return type; Expected int; But got string";
      "[2049] Unbound name: Foo";
      "[2049] Unbound name: foo";
    ]
    (errors_to_string errors)
    "unsaved: compute_tast(class Foo1) should have these errors";

  (* go back to original unsaved content *)
  let (ctx, entry) =
    Provider_context.add_entry_from_file_contents
      ~ctx
      ~path:foo_path
      ~contents:foo_contents
  in
  let { Tast_provider.Compute_tast_and_errors.telemetry; errors; _ } =
    Tast_provider.compute_tast_and_errors_unquarantined ~ctx ~entry
  in
  Asserter.Int_asserter.assert_equals
    2
    (Telemetry_test_utils.int_exn telemetry "get_ast.count")
    "unsaved: compute_tast(class Foo again) should have this many calls to get_ast";
  Asserter.Int_asserter.assert_equals
    0
    (Telemetry_test_utils.int_exn telemetry "disk_cat.count")
    "unsaved: compute_tast(class Foo again) should have this many calls to disk_cat";
  Asserter.String_asserter.assert_list_equals
    [
      "[4110] Invalid return type; Expected int; But got string";
      "[2006] Could not find foo; Did you mean Foo?";
    ]
    (errors_to_string errors)
    "unsaved: compute_tast(class Foo again) should have these errors";

  true

let test_canon_names_internal
    ~(ctx : Provider_context.t)
    ~(id : string)
    ~(canonical : string)
    ~(uncanonical : string) : unit =
  begin
    match Naming_provider.get_type_pos_and_kind ctx canonical with
    | None ->
      Printf.eprintf "Canon[%s]: expected to find symbol '%s'\n" id canonical;
      assert false
    | Some _ -> ()
  end;

  begin
    match Naming_provider.get_type_pos_and_kind ctx uncanonical with
    | None -> ()
    | Some _ ->
      Printf.eprintf
        "Canon[%s]: expected not to find symbol '%s'\n"
        id
        uncanonical;
      assert false
  end;

  begin
    match Naming_provider.get_type_canon_name ctx uncanonical with
    | None ->
      Printf.eprintf
        "Canon[%s]: expected %s to have a canonical name"
        id
        uncanonical;
      assert false
    | Some canon ->
      Asserter.String_asserter.assert_equals
        canonical
        canon
        (Printf.sprintf
           "Canon[%s]: expected '%s' to have canonical name '%s'"
           id
           uncanonical
           canonical)
  end;
  ()

let test_canon_names () =
  Provider_backend.set_local_memory_backend_with_defaults ();
  let { ctx; foo_path; foo_contents; _ } = server_setup () in

  test_canon_names_internal
    ~ctx
    ~id:"ctx"
    ~canonical:"\\Foo"
    ~uncanonical:"\\foo";

  let (ctx, _) =
    Provider_context.add_entry_from_file_contents
      ~ctx
      ~path:foo_path
      ~contents:foo_contents
  in
  test_canon_names_internal
    ~ctx
    ~id:"entry"
    ~canonical:"\\Foo"
    ~uncanonical:"\\foo";

  let foo_contents1 =
    Str.global_replace (Str.regexp "class Foo") "class Foo1" foo_contents
  in
  let (ctx1, _) =
    Provider_context.add_entry_from_file_contents
      ~ctx
      ~path:foo_path
      ~contents:foo_contents1
  in
  test_canon_names_internal
    ~ctx:ctx1
    ~id:"entry1"
    ~canonical:"\\Foo1"
    ~uncanonical:"\\foo1";

  begin
    match Naming_provider.get_type_pos_and_kind ctx1 "\\Foo" with
    | None -> ()
    | Some _ ->
      Printf.eprintf "Canon[entry1b]: expected not to find symbol '\\Foo'\n";
      assert false
  end;

  begin
    match Naming_provider.get_type_canon_name ctx1 "\\foo" with
    | None -> ()
    | Some _ ->
      Printf.eprintf
        "Canon[entry1b]: expected not to find canonical for '\\foo'\n";
      assert false
  end;

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
    ("test_unsaved_symbol_change", test_unsaved_symbol_change);
    ("test_canon_names", test_canon_names);
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
