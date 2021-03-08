(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Decl_export

type args = {
  control_hot_decls: string;
  test_hot_decls: string;
  shallow_decls: bool;
}

let die str =
  prerr_endline str;
  exit 2

let parse_options () =
  let control_hot_decls_path = ref None in
  let test_hot_decls_path = ref None in
  let shallow_decls = ref false in
  let set_shallow_decls are_shallow_decls =
    shallow_decls := are_shallow_decls
  in
  let set_control_hot_decls_path path = control_hot_decls_path := Some path in
  let set_test_hot_decls_path path = test_hot_decls_path := Some path in
  let options =
    [
      ( "--control",
        Arg.String set_control_hot_decls_path,
        "Set path for control hot decls" );
      ( "--test",
        Arg.String set_test_hot_decls_path,
        "Set path for control hot decls" );
    ]
  in
  let usage =
    Printf.sprintf
      "Usage: %s --control <control_hot_decls> --test <test_hot_decls>"
      Sys.argv.(0)
  in
  Arg.parse options (fun _ -> die usage) usage;
  let decls_type_match (decls1, decls2) suffix =
    Filename.check_suffix decls1 suffix && Filename.check_suffix decls2 suffix
  in
  match (!control_hot_decls_path, !test_hot_decls_path) with
  | (Some control_path, Some test_path) ->
    if decls_type_match (control_path, test_path) ".decls" then
      set_shallow_decls false
    else if decls_type_match (control_path, test_path) ".shallowdecls" then
      set_shallow_decls true
    else
      die
      @@ Printf.sprintf
           "Invalid decl comparison between %s and %s!"
           control_path
           test_path;
    {
      control_hot_decls = control_path;
      test_hot_decls = test_path;
      shallow_decls = !shallow_decls;
    }
  | _ -> die usage

let diff decls1_str decls2_str =
  if String.equal decls1_str decls2_str then
    ()
  else
    Tempfile.with_real_tempdir (fun dir ->
        let temp_dir = Path.to_string dir in
        let control =
          Caml.Filename.temp_file ~temp_dir "control_decls" ".txt"
        in
        let test = Caml.Filename.temp_file ~temp_dir "test_decls" ".txt" in
        Disk.write_file ~file:control ~contents:decls1_str;
        Disk.write_file ~file:test ~contents:decls2_str;
        Ppxlib_print_diff.print
          ~diff_command:"diff --label control_decls --label test_decls"
          ~file1:control
          ~file2:test
          ())

let diff_legacy_decls
    (decls1 : saved_legacy_decls) (decls2 : saved_legacy_decls) =
  let decls1_str = show_saved_legacy_decls decls1 ^ "\n" in
  let decls2_str = show_saved_legacy_decls decls2 ^ "\n" in
  diff decls1_str decls2_str

let diff_shallow_decls
    (decls1 : saved_shallow_decls) (decls2 : saved_shallow_decls) =
  let decls1_str = show_saved_shallow_decls decls1 ^ "\n" in
  let decls2_str = show_saved_shallow_decls decls2 ^ "\n" in
  diff decls1_str decls2_str

let () =
  let args = parse_options () in
  let control_decls =
    (* trusting the user to give us a file of the correct format! *)
    SaveStateService.load_contents_unsafe args.control_hot_decls
  in
  let test_decls =
    (* trusting the user to give us a file of the correct format! *)
    SaveStateService.load_contents_unsafe args.test_hot_decls
  in
  if args.shallow_decls then
    diff_shallow_decls control_decls test_decls
  else
    diff_legacy_decls control_decls test_decls
