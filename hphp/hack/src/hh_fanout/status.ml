(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

let go (fanout_calculations : Calculate_fanout.result list) : unit Lwt.t =
  let (fanout_files, explanations) =
    List.fold
      fanout_calculations
      ~init:(Relative_path.Set.empty, Relative_path.Map.empty)
      ~f:(fun acc { Calculate_fanout.fanout_files; explanations; _ } ->
        let (acc_fanout_files, acc_explanations) = acc in
        let acc_fanout_files =
          Relative_path.Set.union acc_fanout_files fanout_files
        in
        let acc_explanations =
          Relative_path.Map.union acc_explanations explanations
        in
        (acc_fanout_files, acc_explanations))
  in
  Relative_path.Map.iter explanations ~f:(fun path explanation ->
      let open Calculate_fanout in
      Tty.cprintf (Tty.Bold Tty.Default) "%s\n" (Relative_path.suffix path);

      let get_symbol_num_files symbol =
        match symbol.outgoing_files with
        | Some outgoing_files ->
          (match Relative_path.Set.cardinal outgoing_files with
          | 1 -> "(1 file)"
          | n -> Printf.sprintf "(%d files)" n)
        | None -> Printf.sprintf "(? files)"
      in
      List.iter explanation.added_symbols ~f:(fun added_symbol ->
          Tty.cprintf
            (Tty.Bold Tty.Green)
            "  A %s"
            added_symbol.symbol_edge.symbol_name;
          Printf.printf " %s\n" (get_symbol_num_files added_symbol));
      List.iter explanation.removed_symbols ~f:(fun removed_symbol ->
          Tty.cprintf
            (Tty.Bold Tty.Red)
            "  D %s"
            removed_symbol.symbol_edge.symbol_name;
          Printf.printf " %s\n" (get_symbol_num_files removed_symbol));
      List.iter explanation.modified_symbols ~f:(fun modified_symbol ->
          Tty.cprintf
            (Tty.Bold Tty.Blue)
            "  M %s"
            modified_symbol.symbol_edge.symbol_name;
          Printf.printf " %s\n" (get_symbol_num_files modified_symbol));
      ());
  Printf.printf "Total files to typecheck: ";
  Tty.cprintf
    (Tty.Bold Tty.Default)
    "%d\n"
    (Relative_path.Set.cardinal fanout_files);
  Lwt.return_unit
