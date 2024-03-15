(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let find ~entry pos ctx : Code_action_types.refactor list =
  if Pos.length pos > 0 then
    let source_text = Ast_provider.compute_source_text ~entry in
    let path = entry.Provider_context.path in
    match Extract_method_traverse.find_candidate ~selection:pos ~entry ctx with
    | Some candidate ->
      let refactor =
        Extract_method_to_refactor.of_candidate ~source_text ~path candidate
      in
      let refactors_from_plugins : Code_action_types.refactor list =
        Extract_method_plugins.find ctx entry path pos candidate
      in
      refactors_from_plugins @ [refactor]
    | None -> []
  else
    []
