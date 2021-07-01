(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let tcopt_with_shallow =
  GlobalOptions.{ default with tco_shallow_class_decl = true }

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

let test_unsaved_symbol_change ~(sqlite : bool) () =
  Provider_backend.set_local_memory_backend_with_defaults ();

  let { Common_setup.ctx; foo_path; foo_contents; _ } =
    Common_setup.setup ~sqlite tcopt_with_shallow
  in

  (* Compute tast as-is *)
  let (ctx, entry) =
    Provider_context.add_or_overwrite_entry_contents
      ~ctx
      ~path:foo_path
      ~contents:foo_contents
  in
  let { Tast_provider.Compute_tast_and_errors.telemetry; errors; _ } =
    Tast_provider.compute_tast_and_errors_unquarantined ~ctx ~entry
  in
  Asserter.Int_asserter.assert_equals
    3
    (Telemetry_test_utils.int_exn telemetry "get_ast.count")
    "unsaved: compute_tast(class Foo) should have this many calls to get_ast";
  Asserter.Int_asserter.assert_equals
    1
    (Telemetry_test_utils.int_exn telemetry "disk_cat.count")
    "unsaved: compute_tast(class Foo) should have this many calls to disk_cat";
  Asserter.String_asserter.assert_list_equals
    [
      "[2006] Could not find `foo`.; Did you mean `~~F~~oo` instead (which only differs by case)?";
      "[4110] Invalid return type; Expected `int`; But got `string`";
    ]
    (errors_to_string errors)
    "unsaved: compute_tast(class Foo) should have these errors";

  (* Make an unsaved change which affects a symbol definition that's used *)
  let foo_contents1 =
    Str.global_replace (Str.regexp "class Foo") "class Foo1" foo_contents
  in
  let (ctx, entry) =
    Provider_context.add_or_overwrite_entry_contents
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
      "[2049] Unbound name: `Foo`";
      "[2049] Unbound name: `foo`";
      "[4110] Invalid return type; Expected `int`; But got `string`";
    ]
    (errors_to_string errors)
    "unsaved: compute_tast(class Foo1) should have these errors";

  (* go back to original unsaved content *)
  let (ctx, entry) =
    Provider_context.add_or_overwrite_entry_contents
      ~ctx
      ~path:foo_path
      ~contents:foo_contents
  in
  let { Tast_provider.Compute_tast_and_errors.telemetry; errors; _ } =
    Tast_provider.compute_tast_and_errors_unquarantined ~ctx ~entry
  in
  Asserter.Int_asserter.assert_equals
    1
    (Telemetry_test_utils.int_exn telemetry "get_ast.count")
    "unsaved: compute_tast(class Foo again) should have this many calls to get_ast";
  Asserter.Int_asserter.assert_equals
    0
    (Telemetry_test_utils.int_exn telemetry "disk_cat.count")
    "unsaved: compute_tast(class Foo again) should have this many calls to disk_cat";
  Asserter.String_asserter.assert_list_equals
    [
      "[2006] Could not find `foo`.; Did you mean `~~F~~oo` instead (which only differs by case)?";
      "[4110] Invalid return type; Expected `int`; But got `string`";
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

let test_canon_names_in_entries () =
  Provider_backend.set_local_memory_backend_with_defaults ();
  let { Common_setup.ctx; foo_path; foo_contents; _ } =
    Common_setup.setup tcopt_with_shallow ~sqlite:false
  in

  test_canon_names_internal
    ~ctx
    ~id:"ctx"
    ~canonical:"\\Foo"
    ~uncanonical:"\\foo";

  let (ctx, _) =
    Provider_context.add_or_overwrite_entry_contents
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
    Provider_context.add_or_overwrite_entry_contents
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

let test_dupe_setup ~(sqlite : bool) =
  Provider_backend.set_local_memory_backend_with_defaults ();
  let setup = Common_setup.setup ~sqlite tcopt_with_shallow in
  let sienv = SearchUtils.quiet_si_env in
  let ctx = setup.Common_setup.ctx in

  (* In the common_setup, 'foo.php' defines symbols Foo,f1,f2.
  Let's make another file 'nonexistent.php' define duplicates,
  but with different capitalization for Foo.
  It gets written to disk because canon-name-lookups have
  to read from disk to resolve canon names. *)
  let contents =
    Str.global_replace
      (Str.regexp "class Foo")
      "class fOo"
      setup.Common_setup.foo_contents
  in
  Disk.write_file
    ~file:(Relative_path.to_absolute setup.Common_setup.nonexistent_path)
    ~contents;
  let dupe =
    ClientIdeIncremental.update_naming_tables_for_changed_file
      ~backend:(Provider_context.get_backend ctx)
      ~popt:(Provider_context.get_popt ctx)
      ~naming_table:setup.Common_setup.naming_table
      ~sienv
      ~path:setup.Common_setup.nonexistent_path
  in

  (* The symbols should still be defined, despite duplicates *)
  Asserter.Relative_path_asserter.assert_option_equals
    (Some setup.Common_setup.foo_path)
    (Naming_provider.get_fun_path ctx "\\f1")
    "dupe: expected f1 to be in original location";
  Asserter.String_asserter.assert_option_equals
    (Some "\\f1")
    (Naming_provider.get_fun_canon_name ctx "\\F1")
    "dupe: expected this canonical spelling of 'F1'";
  Asserter.String_asserter.assert_option_equals
    (Some "\\Foo")
    (Naming_provider.get_type_canon_name ctx "\\foo")
    "dupe: expected this canonical spelling of 'foo'";

  (setup, ctx, dupe)

let test_dupe_then_delete_dupe ~(sqlite : bool) () =
  let (setup, ctx, dupe) = test_dupe_setup ~sqlite in

  (* Now we'll delete 'nonexistent.php'. *)
  Sys_utils.rm_dir_tree
    (Relative_path.to_absolute setup.Common_setup.nonexistent_path);
  let (_unduped : ClientIdeIncremental.changed_file_results) =
    ClientIdeIncremental.update_naming_tables_for_changed_file
      ~backend:(Provider_context.get_backend ctx)
      ~popt:(Provider_context.get_popt ctx)
      ~naming_table:dupe.ClientIdeIncremental.naming_table
      ~sienv:dupe.ClientIdeIncremental.sienv
      ~path:setup.Common_setup.nonexistent_path
  in

  Asserter.Relative_path_asserter.assert_option_equals
    (Some setup.Common_setup.foo_path)
    (Naming_provider.get_fun_path ctx "\\f1")
    "unduped: expected f1 to be here";
  Asserter.String_asserter.assert_option_equals
    (Some "\\f1")
    (Naming_provider.get_fun_canon_name ctx "\\F1")
    "unduped: expected this canonical spelling of 'F1'";
  Asserter.String_asserter.assert_option_equals
    (Some "\\Foo")
    (Naming_provider.get_type_canon_name ctx "\\foo")
    "unduped: expected this canonical spelling for 'foo'";

  true

let test_dupe_then_delete_original ~(sqlite : bool) () =
  let (setup, ctx, dupe) = test_dupe_setup ~sqlite in

  (* Now we'll delete the original 'foo.php'. *)
  Sys_utils.rm_dir_tree (Relative_path.to_absolute setup.Common_setup.foo_path);
  let (_unduped : ClientIdeIncremental.changed_file_results) =
    ClientIdeIncremental.update_naming_tables_for_changed_file
      ~backend:(Provider_context.get_backend ctx)
      ~popt:(Provider_context.get_popt ctx)
      ~naming_table:dupe.ClientIdeIncremental.naming_table
      ~sienv:dupe.ClientIdeIncremental.sienv
      ~path:setup.Common_setup.foo_path
  in

  Asserter.Relative_path_asserter.assert_option_equals
    (Some setup.Common_setup.nonexistent_path)
    (Naming_provider.get_fun_path ctx "\\f1")
    "unduped: expected f1 to be here";
  Asserter.String_asserter.assert_option_equals
    (Some "\\f1")
    (Naming_provider.get_fun_canon_name ctx "\\F1")
    "unduped: expected f1 to be canonical spelling";
  Asserter.String_asserter.assert_option_equals
    (Some "\\fOo")
    (Naming_provider.get_type_canon_name ctx "\\foo")
    "unduped: expected this canonical spelling of 'foo'";
  true

let tests =
  [
    ("test_unsaved_symbol_change_mem", test_unsaved_symbol_change ~sqlite:false);
    ( "test_unsaved_symbol_change_sqlite",
      test_unsaved_symbol_change ~sqlite:true );
    ("test_canon_names_in_entries", test_canon_names_in_entries);
    ("test_dupe_then_delete_dupe_mem", test_dupe_then_delete_dupe ~sqlite:false);
    ( "test_dupe_then_delete_dupe_sqlite",
      test_dupe_then_delete_dupe ~sqlite:true );
    ( "test_dupe_then_delete_original_mem",
      test_dupe_then_delete_original ~sqlite:false );
    ( "test_dupe_then_delete_original_sqlite",
      test_dupe_then_delete_original ~sqlite:true );
  ]

let () =
  EventLogger.init_fake ();
  Typing_deps.trace := false;
  (* trace=false so compute_tast doesn't write to sharedmem *)
  tests |> Unit_test.run_all
