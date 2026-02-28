(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let find ~entry (cursor : Pos.t) ctx =
  let source_text = Ast_provider.compute_source_text ~entry in
  let path = entry.Provider_context.path in
  Inline_method_find_candidate.find_candidate ~cursor ~entry ctx
  |> Option.map ~f:(Inline_method_to_refactor.to_refactor ~path ~source_text)
  |> Option.to_list
