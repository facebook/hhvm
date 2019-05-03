(**
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

let get_type_path type_name =
  Option.map
    ~f:(fun (pos, _) -> FileInfo.get_pos_filename pos)
    (Naming_table.Types.get_pos type_name)

let get_fun_path fun_name =
  Option.map ~f:FileInfo.get_pos_filename (Naming_table.Funs.get_pos fun_name)

let get_const_path fun_name =
  Option.map ~f:FileInfo.get_pos_filename (Naming_table.Consts.get_pos fun_name)

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

let base_disk_state = [
  ("foo.php", foo);
  ("bar.php", bar);
  ("baz.php", baz);
]

let () = Tempfile.with_real_tempdir @@ fun temp_dir ->
  let temp_dir = Path.to_string temp_dir in

  Test.save_state ~enable_reverse_naming_table_fallback:true base_disk_state temp_dir;

  let env = Test.load_state
    ~disk_state:(("qux.php", qux) :: base_disk_state)
    ~local_changes:["qux.php"]
    ~enable_reverse_naming_table_fallback:true
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

  ()
