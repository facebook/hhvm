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
  | Some source_text when Lsp_helpers.lsp_range_is_selection range ->
    let line_to_offset line =
      Full_fidelity_source_text.position_to_offset source_text (line, 0)
    in
    let path = entry.Provider_context.path in
    let selection = Lsp_helpers.lsp_range_to_pos ~line_to_offset path range in
    Extract_classish_find_candidate.find_candidate ~selection ~entry ctx
    |> Option.map
         ~f:(Extract_classish_to_refactors.to_refactors ~path ~source_text)
    |> Option.value ~default:[]
  | Some _
  | None ->
    []
