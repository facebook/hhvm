(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let naming_table_filename = "hh_mini_saved_state_naming.sql"

type args = {
  control: string;
  test: string;
}

let die str =
  prerr_endline str;
  exit 2

let parse_options () =
  let control_path = ref None in
  let test_path = ref None in
  let set_control_path path = control_path := Some path in
  let set_test_path path = test_path := Some path in
  let options =
    [
      ( "--control",
        Arg.String set_control_path,
        "Set path for control saved state" );
      ("--test", Arg.String set_test_path, "Set path for test saved state");
    ]
  in
  let usage =
    Printf.sprintf
      "Usage: %s --control <control_saved_state> --test <test_saved_state>"
      Sys.argv.(0)
  in
  Arg.parse options (fun _ -> die usage) usage;
  match (!control_path, !test_path) with
  | (Some control_path, Some test_path) ->
    { control = control_path; test = test_path }
  | _ -> die usage

let get_test_control_pair args path =
  let control = Filename.concat args.control path in
  let test = Filename.concat args.test path in
  (control, test)

let diff_naming_table args =
  let (control_naming_table, test_naming_table) =
    get_test_control_pair args naming_table_filename
  in
  (* - By default the tool is run on saved-state of the form
        "hack_saved_state/tree/hack/64_distc/<hash>" which saves the
        naming table in sqlite format
     - The saved state "hack_saved_state/tree/hack/naming/<hash>"
       would also have a naming table in the sqlite format.
  *)
  DiffNamingTable.diff control_naming_table test_naming_table

let () =
  let args = parse_options () in
  let naming_tables_and_errors_diff = diff_naming_table args in
  if naming_tables_and_errors_diff then
    die
      (Printf.sprintf
         "Saved states %s and %s are different"
         args.control
         args.test)
