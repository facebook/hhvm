(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let apply_patches_to_string old_content (patches : ServerRenameTypes.patch list)
    : string =
  let buf = Buffer.create (String.length old_content) in
  let patch_list =
    List.sort ~compare:ServerRenameTypes.compare_result patches
  in
  ServerRenameTypes.write_patches_to_buffer buf old_content patch_list;
  Buffer.contents buf

(**
 * Represents the range to underline for a line in a source file.
 *)
type underline = {
  ul_start_column: int;  (** 1-indexed, inclusive *)
  ul_end_column: int;  (** 1-indexed, exclusive *)
}

(** Watch it! ChatGPT wrote this code.

It's a classic implementation for merging half-open intervals.
*)
let merge_underlines underlines =
  let compare u1 u2 =
    let comparison = Int.compare u1.ul_start_column u2.ul_start_column in
    if comparison = 0 then
      Int.compare u1.ul_end_column u2.ul_end_column
    else
      comparison
  in
  let sorted_intervals = List.sort ~compare underlines in
  let rec merge_acc merged_list current_interval = function
    | [] -> current_interval :: merged_list
    | { ul_start_column = x; ul_end_column = y } :: rest_intervals ->
      let { ul_start_column = prev_x; ul_end_column = prev_y } =
        current_interval
      in
      if x <= prev_y then
        merge_acc
          merged_list
          { ul_start_column = prev_x; ul_end_column = max y prev_y }
          rest_intervals
      else
        merge_acc
          (current_interval :: merged_list)
          { ul_start_column = x; ul_end_column = y }
          rest_intervals
  in
  match sorted_intervals with
  | [] -> []
  | hd :: tl -> List.sort ~compare (merge_acc [] hd tl)

let pos_to_underlines source_text (pos : Pos.absolute) : underline list IMap.t =
  let (start_line, start_col, end_line, end_col) =
    Pos.destruct_range_one_based pos
  in

  (* For each line, we'll have to calculate where to put the underline *)
  let num_lines = end_line - start_line + 1 in
  let underlines_per_line =
    List.init num_lines ~f:(fun line_index ->
        let line_number = start_line + line_index in
        let line_len =
          String.length
            (Full_fidelity_source_text.line_text source_text line_number)
        in
        let ul_start_column =
          if line_index = 0 then
            start_col
          else
            0
        in
        let ul_end_column =
          if line_index = num_lines - 1 then
            end_col
          else
            line_len
        in
        (line_number, { ul_start_column; ul_end_column }))
  in
  let underlines_per_line =
    List.fold
      underlines_per_line
      ~init:IMap.empty
      ~f:(fun m (line_number, ul) ->
        IMap.update
          line_number
          (function
            | None -> Some [ul]
            | Some uls -> Some (ul :: uls))
          m)
  in
  underlines_per_line

let underlines_for_line_to_patches (uls : underline list) : string =
  let uls = merge_underlines uls in
  let text =
    let (_, insert_strs) =
      List.fold_left
        uls
        ~init:(1, []) (* columns start at 1! *)
        ~f:(fun (previous_end_column, acc) ul ->
          let { ul_start_column; ul_end_column } = ul in
          let prefix =
            String.make (max 0 (ul_start_column - previous_end_column)) ' '
          in
          let underline = String.make (ul_end_column - ul_start_column) '~' in
          let text = prefix ^ underline in
          (ul_end_column, text :: acc))
    in
    String.concat (List.rev ("\n" :: insert_strs))
  in
  text

let underlines_to_patches
    filename source_text (underlines_per_line : underline list IMap.t) :
    ServerRenameTypes.patch list =
  let text_per_line =
    IMap.map underlines_for_line_to_patches underlines_per_line
  in
  let patch_per_line =
    IMap.mapi
      (fun line_number text ->
        let insert_offset =
          Full_fidelity_source_text.position_to_offset
            source_text
            (line_number + 1, 1)
        in
        let insert_pos =
          Full_fidelity_source_text.relative_pos
            filename
            source_text
            insert_offset
            insert_offset
        in
        let pos = Pos.to_absolute insert_pos in
        ServerRenameTypes.(Replace { pos; text }))
      text_per_line
  in
  IMap.values patch_per_line

let diagnostic_to_underlines
    source_text (diagnostic : ClientIdeMessage.diagnostic) :
    underline list IMap.t list =
  let ClientIdeMessage.{ diagnostic_error; diagnostic_related_hints; _ } =
    diagnostic
  in
  let hint_uls =
    List.map ~f:(pos_to_underlines source_text) diagnostic_related_hints
  in
  let error_uls =
    pos_to_underlines source_text (User_diagnostic.get_pos diagnostic_error)
  in
  error_uls :: hint_uls

let diagnostics_to_underlines
    source_text (diagnostics : ClientIdeMessage.diagnostic list) :
    underline list IMap.t =
  let underlines =
    List.bind diagnostics ~f:(diagnostic_to_underlines source_text)
  in
  List.fold underlines ~init:IMap.empty ~f:(fun acc m ->
      IMap.merge
        (fun _key xs ys ->
          match (xs, ys) with
          | (None, None) -> None
          | (Some xs, None) -> Some xs
          | (None, Some ys) -> Some ys
          | (Some xs, Some ys) -> Some (xs @ ys))
        acc
        m)

let run_exn ctx entry errors =
  let errors = Diagnostics.get_sorted_diagnostic_list errors in
  let error_hashes =
    List.map errors ~f:(fun err ->
        ( User_diagnostic.to_absolute err,
          User_diagnostic.hash_diagnostic_for_saved_state err ))
  in
  let diagnostics = Ide_diagnostics.convert ~ctx ~entry error_hashes in
  let path = entry.Provider_context.path in
  let source_text = entry.Provider_context.source_text |> Option.value_exn in
  let underlines = diagnostics_to_underlines source_text diagnostics in
  let patches = underlines_to_patches path source_text underlines in
  let source_text = Sys_utils.cat @@ Relative_path.to_absolute path in
  let rewritten = apply_patches_to_string source_text patches in
  Printf.printf "%s" rewritten

let run ctx entry errors =
  match run_exn ctx entry errors with
  | exception exn -> print_endline @@ Exn.to_string exn
  | () -> ()
