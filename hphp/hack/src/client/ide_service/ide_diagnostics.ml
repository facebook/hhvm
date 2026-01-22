(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let convert_error ~classish_positions (error, diagnostic_hash) =
  let quickfixes = User_diagnostic.quickfixes error in
  let hint_styles = List.bind ~f:Quickfix.get_hint_styles quickfixes in
  let related_hints =
    List.filter_map
      hint_styles
      ~f:
        Quickfix.(
          function
          | HintStyleSilent _ -> None
          | HintStyleHint p -> Classish_positions.find p classish_positions)
  in
  ClientIdeMessage.
    {
      diagnostic_error = error;
      diagnostic_related_hints = related_hints;
      diagnostic_hash;
    }

(** Convert a list of [Diagnostics.finalized_diagnostic] to [ClientIdeMessage.diagnostic] *)
let convert ~(ctx : Provider_context.t) ~(entry : Provider_context.entry) errors
    =
  let cst = Ast_provider.compute_cst ~ctx ~entry in
  let tree = Provider_context.PositionedSyntaxTree.root cst in

  let classish_positions =
    match entry.Provider_context.source_text with
    | Some source_text ->
      Classish_positions.extract tree source_text entry.Provider_context.path
    | None -> Classish_positions.empty
  in
  let classish_positions =
    Classish_positions.map classish_positions ~f:Pos.to_absolute
  in
  List.map errors ~f:(convert_error ~classish_positions)
