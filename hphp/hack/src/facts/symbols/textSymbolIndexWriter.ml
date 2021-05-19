(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open IndexBuilderTypes
open SearchUtils

let record_in_textfile (filename : string) (symbols : si_scan_result) : unit =
  (* Open a temporary file *)
  let temp_filename = filename ^ ".temp" in
  let channel = Out_channel.create temp_filename in
  (* Write lines to file *)
  List.iter symbols.sisr_capture ~f:(fun symbol ->
      let kindstr = Printf.sprintf "%d" (kind_to_int symbol.sif_kind) in
      Out_channel.output_string channel symbol.sif_name;
      Out_channel.output_string channel " ";
      Out_channel.output_string channel kindstr;
      if SearchUtils.valid_for_acid symbol then
        Out_channel.output_string channel " acid ";
      if SearchUtils.valid_for_acnew symbol then
        Out_channel.output_string channel " acnew ";
      if SearchUtils.valid_for_actype symbol then
        Out_channel.output_string channel " actype ";
      Out_channel.output_string channel "\n");

  (* Clean up *)
  Out_channel.close channel;

  (* Sort the file *)
  let cmdline =
    Printf.sprintf "sort -f \"%s\" -o \"%s\"" temp_filename filename
  in
  let _ = Unix.system cmdline in
  Sys.remove temp_filename
