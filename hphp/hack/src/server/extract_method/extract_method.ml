(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let find ~(range : Lsp.range) ~path ~entry ctx =
  let is_selection =
    Lsp.(
      range.start.line < range.end_.line
      || range.start.line = range.end_.line
         && range.start.character < range.end_.character)
  in
  if is_selection then
    match entry.Provider_context.source_text with
    | Some source_text ->
      let selection =
        let line_to_offset line =
          Full_fidelity_source_text.position_to_offset source_text (line, 0)
        in
        Lsp_helpers.lsp_range_to_pos
          ~line_to_offset
          entry.Provider_context.path
          range
      in
      let candidate_opt =
        Extract_method_traverse.find_candidate ~selection ~entry ctx
      in
      let to_lsp =
        Extract_method_to_lsp.command_or_action_of_candidate ~source_text ~path
      in
      Option.(candidate_opt >>| to_lsp |> to_list)
    | None -> []
  else
    []
