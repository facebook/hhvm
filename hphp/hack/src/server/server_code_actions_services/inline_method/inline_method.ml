(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let find ~entry ~(range : Lsp.range) ctx =
  match entry.Provider_context.source_text with
  | Some source_text ->
    let line_to_offset line =
      Full_fidelity_source_text.position_to_offset source_text (line, 0)
    in
    let path = entry.Provider_context.path in
    let cursor = Lsp_helpers.lsp_range_to_pos ~line_to_offset path range in
    Inline_method_find_candidate.find_candidate ~cursor ~entry ctx
    |> Option.map ~f:(Inline_method_to_refactor.to_refactor ~path ~source_text)
    |> Option.to_list
  | None -> []
