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

let decls_type_match (decls1, decls2) suffix =
  Filename.check_suffix decls1 suffix && Filename.check_suffix decls2 suffix

let legacy decls1 decls2 : bool = decls_type_match (decls1, decls2) ".decls"

let shallow decls1 decls2 : bool =
  decls_type_match (decls1, decls2) ".shallowdecls"

(*
  Print the diff of decls1_str and decls2_str, and
  return whether they differ.
*)
let diff_decls decls1_str decls2_str =
  if String.equal decls1_str decls2_str then (
    Hh_logger.log "The hot decls are identical!";
    false
  ) else (
    Hh_logger.log "The hot decls are different:";
    Tempfile.with_real_tempdir (fun dir ->
        let temp_dir = Path.to_string dir in
        let control =
          Stdlib.Filename.temp_file ~temp_dir "control_decls" ".txt"
        in
        let test = Stdlib.Filename.temp_file ~temp_dir "test_decls" ".txt" in
        Disk.write_file ~file:control ~contents:decls1_str;
        Disk.write_file ~file:test ~contents:decls2_str;
        Ppxlib_print_diff.print
          ~diff_command:"diff --label control_decls --label test_decls"
          ~file1:control
          ~file2:test
          ());
    true
  )

let diff_legacy_decls
    (decls1 : saved_legacy_decls) (decls2 : saved_legacy_decls) =
  Hh_logger.log "Diffing legacy hot decls...";
  let decls1_str = show_saved_legacy_decls decls1 ^ "\n" in
  let decls2_str = show_saved_legacy_decls decls2 ^ "\n" in
  diff_decls decls1_str decls2_str

let diff_shallow_decls
    (decls1 : saved_shallow_decls) (decls2 : saved_shallow_decls) =
  Hh_logger.log "Diffing shallow hot decls...";
  let decls1_str = show_saved_shallow_decls decls1 ^ "\n" in
  let decls2_str = show_saved_shallow_decls decls2 ^ "\n" in
  diff_decls decls1_str decls2_str

let load_hot_decls decls_path =
  Hh_logger.log "Loading hot decls from %s..." decls_path;
  let t = Unix.gettimeofday () in
  (* trusting the user to give us a file of the correct format! *)
  let decls = SaveStateService.load_contents_unsafe decls_path in
  let _ = Hh_logger.log_duration "Loaded hot decls" t in
  decls

let diff control_path test_path =
  let control_decls = load_hot_decls control_path in
  let test_decls = load_hot_decls test_path in
  if shallow control_path test_path then
    diff_shallow_decls control_decls test_decls
  else if legacy control_path test_path then
    diff_legacy_decls control_decls test_decls
  else
    failwith "invalid decl comparison"
