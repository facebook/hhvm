(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

(* These are basically the same tests as in
 * test/integration_ml/saved_state/test_naming_table_sqlite_fallback.ml, but
 * as stripped down to just the basics as possible to make finding the root
 * cause of test failures easier. *)

open Hh_prelude

module Types_pos_asserter = Asserter.Make_asserter (struct
  type t = FileInfo.pos * Naming_types.kind_of_type

  let to_string (pos, type_of_type) =
    Printf.sprintf
      "(%s, %d)"
      (FileInfo.show_pos pos)
      (Naming_types.kind_of_type_to_enum type_of_type)

  let is_equal = Poly.( = )
end)

module Pos_asserter = Asserter.Make_asserter (struct
  type t = FileInfo.pos

  let to_string pos = Printf.sprintf "(%s)" (FileInfo.show_pos pos)

  let is_equal = FileInfo.equal_pos
end)

let files =
  [
    ("foo.php", {|<?hh
    class Foo {}
  |});
    ("bar.php", {|<?hh
    function bar(): void {}
  |});
    ("baz.php", {|<?hh
    type Baz = Foo;
  |});
    ("qux.php", {|<?hh
    const int Qux = 5;
  |});
  ]

let write_and_parse_test_files ctx =
  let files =
    List.map files ~f:(fun (fn, contents) ->
        (Relative_path.from_root ~suffix:fn, contents))
  in
  List.iter files ~f:(fun (fn, contents) ->
      let fn = Path.make (Relative_path.to_absolute fn) in
      let dir = Path.dirname fn in
      Disk.mkdir_p (Path.to_string dir);
      Disk.write_file ~file:(Path.to_string fn) ~contents);
  let (file_infos, errors, failed_parsing) =
    Parsing_service.go
      ctx
      None
      Relative_path.Set.empty
      ~get_next:(MultiWorker.next None (List.map files ~f:fst))
      ParserOptions.default
      ~trace:true
  in
  if not (Errors.is_empty errors) then (
    Errors.iter_error_list
      (fun e ->
        List.iter (Errors.to_list_ e) ~f:(fun (pos, msg) ->
            eprintf
              "%s: %s\n"
              (Pos.string
                 (Pos.to_absolute @@ Pos_or_decl.unsafe_to_raw_pos pos))
              msg))
      errors;
    failwith "Expected no errors from parsing."
  );
  if not (Relative_path.Set.is_empty failed_parsing) then
    failwith "Expected all files to pass parsing.";
  Naming_table.create file_infos

let run_test f =
  Tempfile.with_real_tempdir (fun path ->
      Relative_path.set_path_prefix
        Relative_path.Root
        (Path.concat path "root/");
      let config =
        SharedMem.
          {
            global_size = 1024;
            heap_size = 1024 * 1024;
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
      let ctx =
        Provider_context.empty_for_test
          ~popt:ParserOptions.default
          ~tcopt:TypecheckerOptions.default
          ~deps_mode:Typing_deps_mode.SQLiteMode
      in

      let unbacked_naming_table = write_and_parse_test_files ctx in
      let db_name = Path.to_string (Path.concat path "naming_table.sqlite") in
      let save_results = Naming_table.save unbacked_naming_table db_name in
      Asserter.Int_asserter.assert_equals
        8
        Naming_sqlite.(save_results.files_added + save_results.symbols_added)
        "Expected to add eight rows (four files and four symbols)";
      let (_backed_naming_table : Naming_table.t) =
        Naming_table.load_from_sqlite ctx db_name
      in
      f ();
      true)

let test_dep_graph_blob () =
  run_test (fun () ->
      Tempfile.with_tempdir @@ fun dir ->
      let workers = None in
      let delegate_state = Typing_service_delegate.default in
      let opts = TypecheckerOptions.default in
      let dynamic_view_files = Relative_path.Set.empty in
      let memory_cap = None in
      let check_info =
        Typing_service_types.
          {
            init_id = "123";
            recheck_id = None;
            profile_log = false;
            profile_type_check_duration_threshold = 0.0;
            profile_type_check_memory_threshold_mb = 0;
            profile_type_check_twice = false;
            profile_decling = Typing_service_types.DeclingOff;
          }
      in
      let ctx =
        Provider_context.empty_for_test
          ~popt:ParserOptions.default
          ~tcopt:opts
          ~deps_mode:Typing_deps_mode.SQLiteMode
      in

      (* Check reentrancy *)
      for i = 0 to 2 do
        let (errors, _delegate_state, _telemetry) =
          Typing_check_service.go
            ctx
            workers
            delegate_state
            (Telemetry.create ())
            dynamic_view_files
            [Relative_path.from_root ~suffix:"baz.php"]
            ~memory_cap
            ~longlived_workers:false
            ~check_info
        in

        Asserter.Bool_asserter.assert_equals
          true
          (Errors.is_empty errors)
          "Unexpected type errors";

        Asserter.Int_asserter.assert_equals
          1
          (SharedMem.get_in_memory_dep_table_entry_count ())
          "Expected the correct # of edges in memory after saving dep table blob";

        let filename = Path.(to_string (concat dir "deptable.bin")) in
        let edges =
          SharedMem.save_dep_table_blob
            filename
            "build_revision"
            ~reset_state_after_saving:true
        in

        Asserter.Int_asserter.assert_equals
          2
          edges
          "Expected # of edges to be correct";

        Asserter.Int_asserter.assert_equals
          0
          (SharedMem.get_in_memory_dep_table_entry_count ())
          "Expected 0 edges in memory after saving dep table blob"
      done)

let () = Unit_test.run_all [("test_dep_graph_blob", test_dep_graph_blob)]
