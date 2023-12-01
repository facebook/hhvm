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

let deps_mode = Typing_deps_mode.InMemoryMode None

module Types_pos_asserter = Asserter.Make_asserter (struct
  type t = FileInfo.pos * Naming_types.kind_of_type

  let to_string (pos, kind_of_type) =
    Printf.sprintf
      "(%s, %s)"
      (FileInfo.show_pos pos)
      (Naming_types.show_kind_of_type kind_of_type)

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
    ("qux.php", {|<?hh
    const int Qux = 5;
  |});
    ("corge.php", {|<?hh
    new module Corge {}
  |});
    ( "corge2.php",
      {|<?hh
    // modules are case sensitive, this is a different symbol
    new module corge {}
  |}
    );
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
  let get_next = MultiWorker.next None (List.map files ~f:fst) in
  let file_infos =
    Direct_decl_service.go ctx None ~get_next ~trace:true ~cache_decls:false
  in
  Naming_table.create file_infos

let run_naming_table_test f =
  Tempfile.with_real_tempdir (fun path ->
      Relative_path.set_path_prefix
        Relative_path.Root
        (Path.concat path "root/");
      let config =
        SharedMem.
          {
            global_size = 1024;
            heap_size = 1024 * 1024;
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
      let popt =
        ParserOptions.with_allow_unstable_features ParserOptions.default true
      in
      let tcopt = TypecheckerOptions.default in
      let ctx =
        Provider_context.empty_for_tool
          ~popt
          ~tcopt
          ~backend:(Provider_backend.get ())
          ~deps_mode
      in
      let (_ : SharedMem.handle) = SharedMem.init config ~num_workers:0 in
      let unbacked_naming_table = write_and_parse_test_files ctx in
      let db_name = Path.to_string (Path.concat path "naming_table.sqlite") in
      let save_results = Naming_table.save unbacked_naming_table db_name in
      Asserter.Int_asserter.assert_equals
        12
        Naming_sqlite.(save_results.files_added + save_results.symbols_added)
        "Expected to add 12 rows (6 files and 6 symbols)";

      let ctx_for_sqlite_load =
        Provider_context.empty_for_test ~popt ~tcopt ~deps_mode
      in
      let backed_naming_table =
        Naming_table.load_from_sqlite ctx_for_sqlite_load db_name
      in

      Provider_backend.set_local_memory_backend_with_defaults_for_test ();
      let ctx =
        Provider_context.empty_for_tool
          ~popt
          ~tcopt
          ~backend:(Provider_backend.get ())
          ~deps_mode
      in
      (* load_from_sqlite will call set_naming_db_path for the ctx it's given, but
         here is a fresh ctx with a fresh backend so we have to set it again. *)
      Db_path_provider.set_naming_db_path
        (Provider_context.get_backend ctx)
        (Some (Naming_sqlite.Db_path db_name));
      (try
         f
           ~ctx
           ~unbacked_naming_table
           ~backed_naming_table
           ~db_name
           ~tmp_path:path
       with
      | e ->
        Printf.eprintf
          "NOTE: backend was local-memory for this exception's test run\n";
        raise e);
      Provider_backend.set_shared_memory_backend ();
      let ctx =
        Provider_context.empty_for_tool
          ~popt
          ~tcopt
          ~backend:(Provider_backend.get ())
          ~deps_mode
      in
      (try
         f
           ~ctx
           ~unbacked_naming_table
           ~backed_naming_table
           ~db_name
           ~tmp_path:path
       with
      | e ->
        Printf.eprintf
          "NOTE: backend was shared-memory for this exception's test run\n";
        raise e);
      true)

let test_get_pos () =
  run_naming_table_test
    (fun
      ~ctx
      ~unbacked_naming_table:_
      ~backed_naming_table:_
      ~db_name:_
      ~tmp_path:_
    ->
      Types_pos_asserter.assert_option_equals
        (Some
           ( FileInfo.File
               (FileInfo.Class, Relative_path.from_root ~suffix:"foo.php"),
             Naming_types.TClass ))
        (Naming_provider.get_type_pos_and_kind ctx "\\Foo")
        "Check for class type";
      Pos_asserter.assert_option_equals
        (Some
           (FileInfo.File
              (FileInfo.Fun, Relative_path.from_root ~suffix:"bar.php")))
        (Naming_provider.get_fun_pos ctx "\\bar")
        "Check for function";
      Types_pos_asserter.assert_option_equals
        (Some
           ( FileInfo.File
               (FileInfo.Typedef, Relative_path.from_root ~suffix:"baz.php"),
             Naming_types.TTypedef ))
        (Naming_provider.get_type_pos_and_kind ctx "\\Baz")
        "Check for typedef type";
      Pos_asserter.assert_option_equals
        (Some
           (FileInfo.File
              (FileInfo.Const, Relative_path.from_root ~suffix:"qux.php")))
        (Naming_provider.get_const_pos ctx "\\Qux")
        "Check for const";
      Pos_asserter.assert_option_equals
        (Some
           (FileInfo.File
              (FileInfo.Module, Relative_path.from_root ~suffix:"corge.php")))
        (Naming_provider.get_module_pos ctx "Corge")
        "Check for module")

let test_get_canon_name () =
  run_naming_table_test
    (fun
      ~ctx
      ~unbacked_naming_table:_
      ~backed_naming_table:_
      ~db_name:_
      ~tmp_path:_
    ->
      (* Since we're parsing but not naming, the canon heap must fall back to the
         files on disk, which is the situation we'd be in when loading from a
         saved state. *)
      Asserter.String_asserter.assert_option_equals
        (Some "\\Foo")
        (Naming_provider.get_type_canon_name ctx "\\foo")
        "Check for class canon name";
      Asserter.String_asserter.assert_option_equals
        (Some "\\bar")
        (Naming_provider.get_fun_canon_name ctx "\\bar")
        "Check for function canon name lowercase";
      Asserter.String_asserter.assert_option_equals
        (Some "\\bar")
        (Naming_provider.get_fun_canon_name ctx "\\BAR")
        "Check for function canon name uppercase";
      Asserter.String_asserter.assert_option_equals
        (Some "\\Baz")
        (Naming_provider.get_type_canon_name ctx "\\baz")
        "Check for typedef canon name")

let test_remove () =
  run_naming_table_test
    (fun
      ~ctx:_
      ~unbacked_naming_table
      ~backed_naming_table
      ~db_name:_
      ~tmp_path:_
    ->
      let foo_path = Relative_path.from_root ~suffix:"foo.php" in
      assert (
        Naming_table.get_file_info unbacked_naming_table foo_path
        |> Option.is_some);
      let unbacked_naming_table =
        Naming_table.remove unbacked_naming_table foo_path
      in
      assert (
        Naming_table.get_file_info unbacked_naming_table foo_path
        |> Option.is_none);

      assert (
        Naming_table.get_file_info backed_naming_table foo_path
        |> Option.is_some);
      let backed_naming_table =
        Naming_table.remove backed_naming_table foo_path
      in
      assert (
        Naming_table.get_file_info backed_naming_table foo_path
        |> Option.is_none))

let test_get_sqlite_paths () =
  run_naming_table_test
    (fun ~ctx ~unbacked_naming_table:_ ~backed_naming_table ~db_name ~tmp_path:_
    ->
      let provider_path =
        match
          Db_path_provider.get_naming_db_path (Provider_context.get_backend ctx)
        with
        | None -> None
        | Some (Naming_sqlite.Db_path path) -> Some path
      in
      Asserter.String_asserter.assert_option_equals
        (Some db_name)
        provider_path
        "get_naming_db_path should return the expected value";

      Asserter.String_asserter.assert_option_equals
        (Some db_name)
        (Naming_table.get_forward_naming_fallback_path backed_naming_table)
        "get_forward_naming_fallback_path should return the expected value")

let test_local_changes () =
  run_naming_table_test
    (fun ~ctx ~unbacked_naming_table:_ ~backed_naming_table ~db_name ~tmp_path
    ->
      let a_name = "CONST_IN_A" in

      let a_file = Relative_path.from_root ~suffix:"a.php" in
      let a_pos = FileInfo.File (FileInfo.Const, a_file) in
      let a_file_info =
        FileInfo.
          {
            FileInfo.empty_t with
            ids =
              {
                FileInfo.empty_ids with
                FileInfo.consts = [(a_pos, a_name, None)];
              };
            position_free_decl_hash = Some (Int64.of_int 1234567);
          }
      in
      let backed_naming_table =
        Naming_table.update backed_naming_table a_file a_file_info
      in
      let changes_since_baseline_path =
        Path.concat tmp_path "base_plus_changes" |> Path.to_string
      in
      Naming_table.save_changes_since_baseline
        backed_naming_table
        ~destination_path:changes_since_baseline_path;
      let (changes_since_baseline : Naming_table.changes_since_baseline) =
        Marshal.from_string (Disk.cat changes_since_baseline_path) 0
      in
      Asserter.Relative_path_asserter.assert_list_equals
        [a_file]
        (Naming_table.get_files_changed_since_baseline changes_since_baseline)
        "Expected files changed since baseline to be correct";
      let backed_naming_table' =
        Naming_table.load_from_sqlite_with_changes_since_baseline
          ctx
          changes_since_baseline
          db_name
      in
      let a_file_info' =
        Option.value_exn
          (Naming_table.get_file_info backed_naming_table' a_file)
      in
      Asserter.Bool_asserter.assert_equals
        true
        (FileInfo.equal_hash_type
           a_file_info.FileInfo.position_free_decl_hash
           a_file_info'.FileInfo.position_free_decl_hash)
        "Expected file info to be found in the naming table";
      let a_pos' =
        Option.value_exn (Naming_provider.get_const_pos ctx a_name)
      in
      Asserter.Bool_asserter.assert_equals
        true
        (FileInfo.equal_pos a_pos a_pos')
        "Expected position of constant to be found in the naming table")

let test_context_changes_consts () =
  run_naming_table_test
    (fun
      ~ctx
      ~unbacked_naming_table:_
      ~backed_naming_table:_
      ~db_name:_
      ~tmp_path:_
    ->
      let (ctx, _entry) =
        Provider_context.add_or_overwrite_entry_contents
          ~ctx
          ~path:(Relative_path.from_root ~suffix:"foo.php")
          ~contents:{|<?hh
          class New_qux {}
          |}
      in
      let (ctx, _entry) =
        Provider_context.add_or_overwrite_entry_contents
          ~ctx
          ~path:(Relative_path.from_root ~suffix:"qux.php")
          ~contents:{|<?hh
          const int New_qux = 5;
          |}
      in
      Asserter.Relative_path_asserter.assert_option_equals
        (Some (Relative_path.from_root ~suffix:"qux.php"))
        (Naming_provider.get_const_path ctx "\\New_qux")
        "New const in context should be visible";
      Asserter.Relative_path_asserter.assert_option_equals
        None
        (Naming_provider.get_const_path ctx "\\Qux")
        "Old, deleted const in context should NOT be visible")

let test_context_changes_funs () =
  run_naming_table_test
    (fun
      ~ctx
      ~unbacked_naming_table:_
      ~backed_naming_table:_
      ~db_name:_
      ~tmp_path:_
    ->
      Asserter.String_asserter.assert_option_equals
        (Some "\\bar")
        (Naming_provider.get_fun_canon_name ctx "\\bar")
        "Existing function should be accessible by non-canon name \\bar";
      Asserter.String_asserter.assert_option_equals
        (Some "\\bar")
        (Naming_provider.get_fun_canon_name ctx "\\BAR")
        "Existing function should be accessible by non-canon name \\BAR";

      let (ctx, _entry) =
        Provider_context.add_or_overwrite_entry_contents
          ~ctx
          ~path:(Relative_path.from_root ~suffix:"foo.php")
          ~contents:{|<?hh
          class bar {}
          |}
      in
      let (ctx, _entry) =
        Provider_context.add_or_overwrite_entry_contents
          ~ctx
          ~path:(Relative_path.from_root ~suffix:"bar.php")
          ~contents:{|<?hh
          function new_bar(): void {}
          |}
      in
      Asserter.Relative_path_asserter.assert_option_equals
        (Some (Relative_path.from_root ~suffix:"bar.php"))
        (Naming_provider.get_fun_path ctx "\\new_bar")
        "New function in context should be visible";
      Asserter.Relative_path_asserter.assert_option_equals
        None
        (Naming_provider.get_fun_path ctx "\\bar")
        "Old, deleted function in context should NOT be visible";

      Asserter.String_asserter.assert_option_equals
        (Some "\\new_bar")
        (Naming_provider.get_fun_canon_name ctx "\\NeW_bAr")
        "New function in context should be accessible by canon name";

      (* NB: under shared-memory provider, the following two tests aren't
         useful. Sharedmem doesn't suppress canonical lookup results that
         have been overridden by the context. (That's because sharedmem only
         gives us back the canonical name, not the path where that canonical
         name was defined, and without paths we can't tell whether it's been
         overridden by context). For the sharedmem case, the caller is expected
         to manually remove any old reverse-naming-table entries before calling
         into the naming provider -- something that this test doesn't do.
         Hence why it gives incorrect answers. *)
      let expected =
        match Provider_context.get_backend ctx with
        | Provider_backend.Shared_memory ->
          Some "\\bar" (* because the caller (us) is expected to clean up *)
        | _ -> None
      in
      Asserter.String_asserter.assert_option_equals
        expected
        (Naming_provider.get_fun_canon_name ctx "\\bar")
        "Old function in context should NOT be accessible by non-canon name \\bar";
      Asserter.String_asserter.assert_option_equals
        expected
        (Naming_provider.get_fun_canon_name ctx "\\BAR")
        "Old function in context should NOT be accessible by non-canon name \\BAR";
      Asserter.String_asserter.assert_option_equals
        expected
        (Naming_provider.get_fun_canon_name ctx "\\BaR")
        "Old function in context should NOT be accessible by non-canon name \\BaR";
      ())

let test_context_changes_classes () =
  run_naming_table_test
    (fun
      ~ctx
      ~unbacked_naming_table:_
      ~backed_naming_table:_
      ~db_name:_
      ~tmp_path:_
    ->
      Asserter.String_asserter.assert_option_equals
        (Some "\\Foo")
        (Naming_provider.get_type_canon_name ctx "\\Foo")
        "Existing class should be accessible by non-canon name \\Foo";
      Asserter.String_asserter.assert_option_equals
        (Some "\\Foo")
        (Naming_provider.get_type_canon_name ctx "\\FOO")
        "Existing class should be accessible by non-canon name \\FOO";

      let (ctx, _entry) =
        Provider_context.add_or_overwrite_entry_contents
          ~ctx
          ~path:(Relative_path.from_root ~suffix:"foo.php")
          ~contents:{|<?hh
          class NewFoo {}
          |}
      in
      Asserter.Relative_path_asserter.assert_option_equals
        (Some (Relative_path.from_root ~suffix:"foo.php"))
        (Naming_provider.get_class_path ctx "\\NewFoo")
        "New class in context should be visible";
      Asserter.Relative_path_asserter.assert_option_equals
        None
        (Naming_provider.get_class_path ctx "\\Foo")
        "Old class in context should NOT be visible";

      Asserter.String_asserter.assert_option_equals
        (Some "\\NewFoo")
        (Naming_provider.get_type_canon_name ctx "\\NEWFOO")
        "New class in context should be accessible by canon name";

      (* NB: under shared-memory provider, the following two tests aren't
         useful. Sharedmem doesn't suppress canonical lookup results that
         have been overridden by the context. (That's because sharedmem only
         gives us back the canonical name, not the path where that canonical
         name was defined, and without paths we can't tell whether it's been
         overridden by context). For the sharedmem case, the caller is expected
         to manually remove any old reverse-naming-table entries before calling
         into the naming provider -- something that this test doesn't do.
         Hence why it gives incorrect answers. *)
      let expected =
        match Provider_context.get_backend ctx with
        | Provider_backend.Shared_memory ->
          Some "\\Foo" (* because the caller (us) is expected to clean up *)
        | _ -> None
      in
      Asserter.String_asserter.assert_option_equals
        expected
        (Naming_provider.get_type_canon_name ctx "\\Foo")
        "Old class in context should NOT be accessible by non-canon name \\Foo";
      Asserter.String_asserter.assert_option_equals
        expected
        (Naming_provider.get_type_canon_name ctx "\\FOO")
        "Old class in context should NOT be accessible by non-canon name \\FOO";
      Asserter.String_asserter.assert_option_equals
        expected
        (Naming_provider.get_type_canon_name ctx "\\FoO")
        "Old class in context should NOT be accessible by non-canon name \\FoO";
      ())

let test_context_changes_modules () =
  run_naming_table_test
    (fun
      ~ctx
      ~unbacked_naming_table:_
      ~backed_naming_table:_
      ~db_name:_
      ~tmp_path:_
    ->
      Asserter.Relative_path_asserter.assert_option_equals
        (Some (Relative_path.from_root ~suffix:"corge.php"))
        (Naming_provider.get_module_path ctx "Corge")
        "Existing module Corge should be in corge.php";
      Asserter.Relative_path_asserter.assert_option_equals
        (Some (Relative_path.from_root ~suffix:"corge2.php"))
        (Naming_provider.get_module_path ctx "corge")
        "Existing module corge (lowercase) should be in corge2.php";
      let (ctx, _entry) =
        Provider_context.add_or_overwrite_entry_contents
          ~ctx
          ~path:(Relative_path.from_root ~suffix:"corge.php")
          ~contents:{|<?hh
          |}
      in
      Asserter.Relative_path_asserter.assert_option_equals
        None
        (Naming_provider.get_module_path ctx "Corge")
        "module Corge should be deleted";
      Asserter.Relative_path_asserter.assert_option_equals
        (Some (Relative_path.from_root ~suffix:"corge2.php"))
        (Naming_provider.get_module_path ctx "corge")
        "Existing module corge (lowercase) should be in corge2.php";
      ())

let test_context_changes_typedefs () =
  run_naming_table_test
    (fun
      ~ctx
      ~unbacked_naming_table:_
      ~backed_naming_table:_
      ~db_name:_
      ~tmp_path:_
    ->
      Asserter.String_asserter.assert_option_equals
        (Some "\\Baz")
        (Naming_provider.get_type_canon_name ctx "\\Baz")
        "Existing typedef should be accessible by non-canon name \\Baz";
      Asserter.String_asserter.assert_option_equals
        (Some "\\Baz")
        (Naming_provider.get_type_canon_name ctx "\\BAZ")
        "Existing typedef should be accessible by non-canon name \\BAZ";

      let (ctx, _entry) =
        Provider_context.add_or_overwrite_entry_contents
          ~ctx
          ~path:(Relative_path.from_root ~suffix:"baz.php")
          ~contents:{|<?hh
          type NewBaz = Foo;
          |}
      in
      Asserter.Relative_path_asserter.assert_option_equals
        (Some (Relative_path.from_root ~suffix:"baz.php"))
        (Naming_provider.get_typedef_path ctx "\\NewBaz")
        "New typedef in context should be visible";
      Asserter.Relative_path_asserter.assert_option_equals
        None
        (Naming_provider.get_typedef_path ctx "\\Baz")
        "Old typedef in context should NOT be visible";

      Asserter.String_asserter.assert_option_equals
        (Some "\\NewBaz")
        (Naming_provider.get_type_canon_name ctx "\\NEWBAZ")
        "New typedef in context should be accessible by canon name";

      (* NB: under shared-memory provider, the following two tests aren't
         useful. Sharedmem doesn't suppress canonical lookup results that
         have been overridden by the context. (That's because sharedmem only
         gives us back the canonical name, not the path where that canonical
         name was defined, and without paths we can't tell whether it's been
         overridden by context). For the sharedmem case, the caller is expected
         to manually remove any old reverse-naming-table entries before calling
         into the naming provider -- something that this test doesn't do.
         Hence why it gives incorrect answers. *)
      let expected =
        match Provider_context.get_backend ctx with
        | Provider_backend.Shared_memory ->
          Some "\\Baz" (* because the caller (us) is expected to clean up *)
        | _ -> None
      in
      Asserter.String_asserter.assert_option_equals
        expected
        (Naming_provider.get_type_canon_name ctx "\\Baz")
        "Old typedef in context should NOT be accessible by non-canon name \\Baz";
      Asserter.String_asserter.assert_option_equals
        expected
        (Naming_provider.get_type_canon_name ctx "\\BAZ")
        "Old typedef in context should NOT be accessible by non-canon name \\BAZ";
      Asserter.String_asserter.assert_option_equals
        expected
        (Naming_provider.get_type_canon_name ctx "\\BaZ")
        "Old typedef in context should NOT be accessible by non-canon name \\BaZ";
      ())

let test_naming_table_hash () =
  List.iter [0; -1; 1; 200; -200; Int.max_value; Int.min_value] ~f:(fun i ->
      let dep = Typing_deps.Dep.of_debug_string (string_of_int i) in
      let hash = Typing_deps.Dep.to_int64 dep in
      (* "%16x" on a negative integer will produce a hex version as if it were unsigned, e.g. -2 is printed as 7ffffffffffffffe rather than -0000000000000002. *)
      let i_str = Printf.sprintf "0x%016x" i in
      let hash_str = Printf.sprintf "0x%016Lx" hash in
      Asserter.String_asserter.assert_equals
        i_str
        hash_str
        "Expected 64bit hash to be same as int hash");

  let foo_dep = Typing_deps.Dep.Type "\\Foo" in
  let foo_dep_hash =
    Typing_deps.Dep.make foo_dep |> Typing_deps.Dep.to_hex_string
  in
  Asserter.String_asserter.assert_equals
    "0x4cd17f7c3d7b6feb"
    foo_dep_hash
    "Expected foo dep hash to be correct (this test must be updated if the hashing logic changes)";

  true

let test_naming_table_query_by_dep_hash () =
  run_naming_table_test
    (fun
      ~ctx
      ~unbacked_naming_table:_
      ~backed_naming_table
      ~db_name:_
      ~tmp_path:_
    ->
      let db_path =
        Db_path_provider.get_naming_db_path (Provider_context.get_backend ctx)
      in
      let db_path = Option.value_exn db_path in
      Asserter.Relative_path_asserter.assert_option_equals
        (Some (Relative_path.from_root ~suffix:"qux.php"))
        (Typing_deps.Dep.GConst "\\Qux"
        |> Typing_deps.Dep.make
        |> Naming_sqlite.get_path_by_64bit_dep db_path
        |> Option.map ~f:fst)
        "Look up const by dep hash should return file path";
      Asserter.Relative_path_asserter.assert_option_equals
        None
        (Typing_deps.Dep.GConst "\\Nonexistent"
        |> Typing_deps.Dep.make
        |> Naming_sqlite.get_path_by_64bit_dep db_path
        |> Option.map ~f:fst)
        "Look up non-existent const by dep hash should return nothing";

      Asserter.Relative_path_asserter.assert_option_equals
        (Some (Relative_path.from_root ~suffix:"bar.php"))
        (Typing_deps.Dep.Fun "\\bar"
        |> Typing_deps.Dep.make
        |> Naming_sqlite.get_path_by_64bit_dep db_path
        |> Option.map ~f:fst)
        "Look up fun by dep hash should return file path";
      Asserter.Relative_path_asserter.assert_option_equals
        None
        (Typing_deps.Dep.Fun "\\nonexistent"
        |> Typing_deps.Dep.make
        |> Naming_sqlite.get_path_by_64bit_dep db_path
        |> Option.map ~f:fst)
        "Look up non-existent fun by dep hash should return nothing";

      Asserter.Relative_path_asserter.assert_option_equals
        (Some (Relative_path.from_root ~suffix:"foo.php"))
        (Typing_deps.Dep.Type "\\Foo"
        |> Typing_deps.Dep.make
        |> Naming_sqlite.get_path_by_64bit_dep db_path
        |> Option.map ~f:fst)
        "Look up class by dep hash should return file path";
      Asserter.Relative_path_asserter.assert_option_equals
        None
        (Typing_deps.Dep.Type "\\nonexistent"
        |> Typing_deps.Dep.make
        |> Naming_sqlite.get_path_by_64bit_dep db_path
        |> Option.map ~f:fst)
        "Look up non-existent class by dep hash should return nothing";

      Asserter.Relative_path_asserter.assert_option_equals
        (Some (Relative_path.from_root ~suffix:"baz.php"))
        (Typing_deps.Dep.Type "\\Baz"
        |> Typing_deps.Dep.make
        |> Naming_sqlite.get_path_by_64bit_dep db_path
        |> Option.map ~f:fst)
        "Look up class by dep hash should return file path";
      Asserter.Relative_path_asserter.assert_option_equals
        None
        (Typing_deps.Dep.Type "\\nonexistent"
        |> Typing_deps.Dep.make
        |> Naming_sqlite.get_path_by_64bit_dep db_path
        |> Option.map ~f:fst)
        "Look up non-existent typedef by dep hash should return nothing";

      Asserter.Relative_path_asserter.assert_list_equals
        [Relative_path.from_root ~suffix:"qux.php"]
        (Typing_deps.Dep.GConst "\\Qux"
        |> Typing_deps.Dep.make
        |> Typing_deps.DepSet.singleton
        |> Naming_table.get_64bit_dep_set_files backed_naming_table
        |> Relative_path.Set.elements)
        "Bulk lookup for const should be correct";
      Asserter.Relative_path_asserter.assert_list_equals
        [Relative_path.from_root ~suffix:"bar.php"]
        (Typing_deps.Dep.Fun "\\bar"
        |> Typing_deps.Dep.make
        |> Typing_deps.DepSet.singleton
        |> Naming_table.get_64bit_dep_set_files backed_naming_table
        |> Relative_path.Set.elements)
        "Bulk lookup for fun should be correct";
      Asserter.Relative_path_asserter.assert_list_equals
        [Relative_path.from_root ~suffix:"baz.php"]
        (Typing_deps.Dep.Type "\\Baz"
        |> Typing_deps.Dep.make
        |> Typing_deps.DepSet.singleton
        |> Naming_table.get_64bit_dep_set_files backed_naming_table
        |> Relative_path.Set.elements)
        "Bulk lookup for class should be correct";

      Asserter.Relative_path_asserter.assert_list_equals
        [
          Relative_path.from_root ~suffix:"bar.php";
          Relative_path.from_root ~suffix:"baz.php";
          Relative_path.from_root ~suffix:"qux.php";
        ]
        (Typing_deps.DepSet.make ()
        |> Typing_deps.DepSet.union
             (Typing_deps.Dep.GConst "\\Qux"
             |> Typing_deps.Dep.make
             |> Typing_deps.DepSet.singleton)
        |> Typing_deps.DepSet.union
             (Typing_deps.Dep.Fun "\\bar"
             |> Typing_deps.Dep.make
             |> Typing_deps.DepSet.singleton)
        |> Typing_deps.DepSet.union
             (Typing_deps.Dep.Type "\\Baz"
             |> Typing_deps.Dep.make
             |> Typing_deps.DepSet.singleton)
        |> Naming_table.get_64bit_dep_set_files backed_naming_table
        |> Relative_path.Set.elements)
        "Bulk lookup for multiple elements should be correct";

      (* Simulate moving \Baz from baz.php to bar.php. *)
      let baz_file_info = FileInfo.empty_t in
      let bar_file_info =
        {
          (* Might raise {!Naming_table.File_info_not_found} *)
          (Naming_table.get_file_info_exn
             backed_naming_table
             (Relative_path.from_root ~suffix:"bar.php"))
          with
          FileInfo.ids =
            {
              FileInfo.empty_ids with
              FileInfo.classes =
                [
                  ( FileInfo.File
                      (FileInfo.Class, Relative_path.from_root ~suffix:"bar.php"),
                    "\\Baz",
                    None );
                ];
            };
        }
      in
      let new_naming_table = backed_naming_table in
      let new_naming_table =
        Naming_table.update
          new_naming_table
          (Relative_path.from_root ~suffix:"baz.php")
          baz_file_info
      in
      let new_naming_table =
        Naming_table.update
          new_naming_table
          (Relative_path.from_root ~suffix:"bar.php")
          bar_file_info
      in
      Asserter.Relative_path_asserter.assert_list_equals
        [Relative_path.from_root ~suffix:"bar.php"]
        (Typing_deps.Dep.Type "\\Baz"
        |> Typing_deps.Dep.make
        |> Typing_deps.DepSet.singleton
        |> Naming_table.get_64bit_dep_set_files new_naming_table
        |> Relative_path.Set.elements)
        "\\Baz should now be located in bar.php";

      ())

let () =
  let config =
    SharedMem.
      {
        global_size = 1024;
        heap_size = 1024 * 1024;
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
  EventLogger.init_fake ();
  Hh_logger.Level.set_min_level_stderr Hh_logger.Level.Warn;
  Unit_test.run_all
    [
      ("test_get_pos", test_get_pos);
      ("test_get_canon_name", test_get_canon_name);
      ("test_remove", test_remove);
      ("test_get_sqlite_paths", test_get_sqlite_paths);
      ("test_local_changes", test_local_changes);
      ("test_context_changes_consts", test_context_changes_consts);
      ("test_context_changes_funs", test_context_changes_funs);
      ("test_context_changes_classes", test_context_changes_classes);
      ("test_context_changes_typedefs", test_context_changes_typedefs);
      ("test_context_changes_modules", test_context_changes_modules);
      ("test_naming_table_hash", test_naming_table_hash);
      ( "test_naming_table_query_by_dep_hash",
        test_naming_table_query_by_dep_hash );
    ]
