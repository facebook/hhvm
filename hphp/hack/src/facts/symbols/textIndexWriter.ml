(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)
open SearchUtils

let record_in_textfile
    (filename: string)
    (symbols: si_results): unit =

  (* Open a temporary file *)
  let open Core_kernel in
  let temp_filename = filename ^ ".temp" in
  let channel = Out_channel.create temp_filename in

  (* Write lines to file *)
  List.iter symbols ~f:(fun symbol -> begin
    let kindstr = Printf.sprintf "%d" (kind_to_int symbol.si_kind) in
      Out_channel.output_string channel symbol.si_name;
      Out_channel.output_string channel " ";
      Out_channel.output_string channel kindstr;
      Out_channel.output_string channel "\n";
  end);

  (* Clean up *)
  Out_channel.close channel;

  (* Sort the file *)
  let cmdline = Printf.sprintf "sort -f \"%s\" -o \"%s\""
    temp_filename filename in
  let _ = Unix.system cmdline in
  Sys.remove filename;
;;
