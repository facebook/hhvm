(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

(* These are basically the same tests as in test/unit/naming/naming_table_tests.ml,
 * but these ones go through the entire server initialization process to make
 * sure we set everything up correctly. *)

open Core_kernel
module Test = Integration_test_base

module FileInfoComparator = struct
  type t = FileInfo.t

  let to_string = FileInfo.show

  let is_equal left right =
    (* We use show here in order to avoid matching on private members of Pos.t
     * records. *)
    FileInfo.show left = FileInfo.show right
end

module FileInfoAsserter = Asserter.Make_asserter (FileInfoComparator)

let get_type_path type_name =
  Option.map
    ~f:(fun (pos, _) -> FileInfo.get_pos_filename pos)
    (Naming_table.Types.get_pos type_name)

let get_fun_path fun_name =
  Option.map ~f:FileInfo.get_pos_filename (Naming_table.Funs.get_pos fun_name)

let get_const_path fun_name =
  Option.map
    ~f:FileInfo.get_pos_filename
    (Naming_table.Consts.get_pos fun_name)

let make_full_pos path_name (start_lnum, start_cnum) (end_lnum, end_cnum) =
  Pos.make_from_lexing_pos
    (Relative_path.from_root path_name)
    Lexing.
      {
        pos_fname = path_name;
        pos_lnum = start_lnum;
        pos_bol = 1;
        pos_cnum = start_cnum;
      }
    Lexing.
      {
        pos_fname = path_name;
        pos_lnum = end_lnum;
        pos_bol = 1;
        pos_cnum = end_cnum;
      }

let foo = {|<?hh
  class Foo {}
|}

let bar = {|<?hh
  function bar(): Foo {
    return new Foo();
  }
|}

let baz = {|<?hh
  const int BAZCONST = 5;
|}

let qux = {|<?hh
  function qux(): int {
    return 5;
  }
|}

let base_disk_state = [("foo.php", foo); ("bar.php", bar); ("baz.php", baz)]

let test () =
  Tempfile.with_real_tempdir
  @@ fun temp_dir ->
  let temp_dir = Path.to_string temp_dir in
  Test.save_state ~enable_naming_table_fallback:true base_disk_state temp_dir;

  let env =
    Test.load_state
      ~disk_state:(("qux.php", qux) :: base_disk_state)
      ~local_changes:["qux.php"]
      ~enable_naming_table_fallback:true
      temp_dir
  in
  Test.assert_no_errors env;

  Asserter.Relative_path_asserter.assert_option_equals
    (Some (Relative_path.from_root "foo.php"))
    (get_type_path "\\Foo")
    "Basic test to get a type from the saved state";
  Asserter.Relative_path_asserter.assert_option_equals
    (Some (Relative_path.from_root "bar.php"))
    (get_fun_path "\\bar")
    "Basic test to get a function from the saved state";
  Asserter.Relative_path_asserter.assert_option_equals
    (Some (Relative_path.from_root "baz.php"))
    (get_const_path "\\BAZCONST")
    "Basic test to get a const from from the saved state";
  Asserter.Relative_path_asserter.assert_option_equals
    (Some (Relative_path.from_root "qux.php"))
    (get_fun_path "\\qux")
    "Basic test to get a function from disk";

  Asserter.String_asserter.assert_option_equals
    (Some "\\Foo")
    (Naming_table.Types.get_canon_name "\\foo")
    "Basic test to get a canon name for a type defined in the saved state";
  Asserter.String_asserter.assert_option_equals
    (Some "\\bar")
    (Naming_table.Funs.get_canon_name "\\bar")
    "Basic test to get a canon name for a function defined in the saved state";
  Asserter.String_asserter.assert_option_equals
    (Some "\\qux")
    (Naming_table.Funs.get_canon_name "\\qux")
    "Basic test to get a canon name for a function defined only on disk";

  let () =
    FileInfo.(
      Ast_provider.local_changes_push_stack ();

      (* We make a table of on-disk hashes instead of hardcoding them in the test
       * data so that we don't need to update them every time we change the AST
       * schema. *)
      let hashes =
        List.map (("qux.php", "") :: base_disk_state) ~f:(fun (name, _) ->
            let relative_path = Relative_path.from_root name in
            let ast = Ast_provider.get_ast relative_path in
            (name, Nast.generate_ast_decl_hash ast))
        |> SMap.of_list
      in
      Ast_provider.local_changes_pop_stack ();
      let naming_table = env.ServerEnv.naming_table in
      Naming_table.assert_is_backed naming_table true;
      let empty = { empty_t with file_mode = Some Mstrict; comments = None } in
      FileInfoAsserter.assert_option_equals
        (Some
           {
             empty with
             hash = SMap.get "foo.php" hashes;
             classes =
               [(File (Class, Relative_path.from_root "foo.php"), "\\Foo")];
           })
        (Naming_table.get_file_info
           naming_table
           (Relative_path.from_root "foo.php"))
        "Basic test to get a file info for a relative path.";
      FileInfoAsserter.assert_option_equals
        (Some
           {
             empty with
             hash = SMap.get "bar.php" hashes;
             funs = [(File (Fun, Relative_path.from_root "bar.php"), "\\bar")];
           })
        (Naming_table.get_file_info
           naming_table
           (Relative_path.from_root "bar.php"))
        "Basic test to get a file info for a relative path.";
      FileInfoAsserter.assert_option_equals
        (Some
           {
             empty with
             hash = SMap.get "baz.php" hashes;
             consts =
               [
                 (File (Const, Relative_path.from_root "baz.php"), "\\BAZCONST");
               ];
           })
        (Naming_table.get_file_info
           naming_table
           (Relative_path.from_root "baz.php"))
        "Basic test to get a file info for a relative path.";
      FileInfoAsserter.assert_option_equals
        (Some
           {
             empty with
             hash = SMap.get "qux.php" hashes;
             funs = [(Full (make_full_pos "qux.php" (2, 12) (2, 15)), "\\qux")];
           })
        (Naming_table.get_file_info
           naming_table
           (Relative_path.from_root "qux.php"))
        "Basic test to get a file info for a relative path.";
      FileInfoAsserter.assert_option_equals
        None
        (Naming_table.get_file_info
           naming_table
           (Relative_path.from_root "does_not_exist.php"))
        "Basic test to get a file info for a relative path that doesn't exist.")
  in
  ()
