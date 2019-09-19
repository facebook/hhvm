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
  let deferred = Deferred_decl.get ~f:(fun d -> d) in
  Asserter.Int_asserter.assert_equals
    count
    (List.length deferred)
    "The number of deferred items should match the expected value"

let test_deferred_decl_add () =
  Deferred_decl.reset ~enable:true;
  ensure_count 0;

  Deferred_decl.add (Relative_path.create Relative_path.Dummy "foo");
  ensure_count 1;

  Deferred_decl.add (Relative_path.create Relative_path.Dummy "foo");
  ensure_count 1;

  Deferred_decl.add (Relative_path.create Relative_path.Dummy "bar");
  ensure_count 2;

  Deferred_decl.reset ~enable:true;
  ensure_count 0;

  true

let ensure_threshold ~(threshold : int) ~(limit : int) ~(expected : int) : unit
    =
  Deferred_decl.reset ~enable:true;
  ensure_count 0;

  let deferred_count = ref 0 in
  for i = 1 to limit do
    let path = Printf.sprintf "foo-%d" i in
    let relative_path = Relative_path.create Relative_path.Dummy path in
    try Deferred_decl.should_defer ~d:relative_path ~threshold
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
    public function foo (Bar $b): int {
      return $b->toString();
    }
  }
|}

let bar_contents =
  {|<?hh //strict
  class Bar {
    public function toString() : string {
      return "bar";
    }
  }
|}

(* In this test, we wish to establish that we enable deferring type checking
  for files that have undeclared dependencies, UNLESS we've already deferred
  those files a certain number of times. *)
let test_process_file_deferring () =
  Deferred_decl.reset ~enable:true;

  (* Set up a simple fake repo *)
  Disk.mkdir_p "/fake/root/";
  Relative_path.set_path_prefix Relative_path.Root (Path.make "/fake/root/");

  (* We'll need to parse these files in order to create a naming table, which
    will be used for look up of symbols in type checking. *)
  Disk.write_file ~file:"/fake/root/Foo.php" ~contents:foo_contents;
  Disk.write_file ~file:"/fake/root/Bar.php" ~contents:bar_contents;
  let foo_path =
    Relative_path.create Relative_path.Root "/fake/root/Foo.php"
  in
  let bar_path =
    Relative_path.create Relative_path.Root "/fake/root/Bar.php"
  in
  (* The parsing service needs shared memory to be set up *)
  let config =
    SharedMem.
      {
        global_size = 1024;
        heap_size = 1024 * 4;
        dep_table_pow = 16;
        hash_table_pow = 10;
        shm_dirs = [];
        shm_min_avail = 0;
        log_level = 0;
        sample_rate = 0.0;
      }
  in
  let (_ : SharedMem.handle) = SharedMem.init config ~num_workers:0 in
  (* Parsing produces the file infos that the naming table module can use
    to construct the forward naming table (files-to-symbols) *)
  let (file_infos, _errors, _failed_parsing) =
    Parsing_service.go
      None
      Relative_path.Set.empty
      ~get_next:(MultiWorker.next None [foo_path; bar_path])
      ParserOptions.default
      ~trace:true
  in
  let naming_table = Naming_table.create file_infos in
  (* Construct the reverse naming table (symbols-to-files) *)
  let fast = Naming_table.to_fast naming_table in
  Relative_path.Map.iter fast ~f:(fun name info ->
      let {
        FileInfo.n_classes = classes;
        n_types = typedefs;
        n_funs = funs;
        n_consts = consts;
      } =
        info
      in
      NamingGlobal.ndecl_file_fast name ~funs ~classes ~typedefs ~consts);

  (* Construct one instance of file type check computation. Class \Foo depends
    on class \Bar being declared, so processing \Foo once should result in
    two computations:
      - a declaration of \Bar
      - a (deferred) type check of \Foo *)
  let dynamic_view_files = Relative_path.Set.empty in
  let opts =
    GlobalNamingOptions.set
      GlobalOptions.
        { default with tco_defer_class_declaration_threshold = Some 1 };
    GlobalNamingOptions.get ()
  in
  let errors = Errors.empty in
  let file =
    Typing_check_service.
      {
        path = foo_path;
        names = Relative_path.Map.find foo_path fast;
        deferred_count = 0;
      }
  in
  (* Finally, this is what all the setup was for: process this file *)
  let (_errors, file_computations) =
    Typing_check_service.process_file dynamic_view_files opts errors file
  in
  Asserter.Int_asserter.assert_equals
    2
    (List.length file_computations)
    "Should be two file computations";

  (* Validate the deferred type check computation *)
  let found_check =
    List.exists file_computations ~f:(fun file_computation ->
        match file_computation with
        | Typing_check_service.(Check { path; names = _names; deferred_count })
          ->
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
    List.exists file_computations ~f:(fun file_computation ->
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

let test_should_enable_deferring () =
  Relative_path.set_path_prefix Relative_path.Root (Path.make "/fake/www");

  let opts =
    GlobalOptions.
      { default with tco_max_times_to_defer_type_checking = Some 2 }
  in
  let file =
    Typing_check_service.
      {
        path = Relative_path.create Relative_path.Root "/fake/www/Foo.php";
        names = FileInfo.empty_names;
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

let tests =
  [
    ("test_deferred_decl_add", test_deferred_decl_add);
    ("test_deferred_decl_should_defer", test_deferred_decl_should_defer);
    ("test_process_file_deferring", test_process_file_deferring);
    ("test_dmesg_parser", test_dmesg_parser);
    ("test_should_enable_deferring", test_should_enable_deferring);
  ]

let () = Unit_test.run_all tests
