(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let find ~entry pos ctx =
  if Pos.length pos > 0 then
    let source_text = Ast_provider.compute_source_text ~entry in
    let path = entry.Provider_context.path in
    Extract_classish_find_candidate.find_candidate ~selection:pos entry ctx
    |> Option.map
         ~f:(Extract_classish_to_refactors.to_refactors source_text path)
    |> Option.value ~default:[]
  else
    []
