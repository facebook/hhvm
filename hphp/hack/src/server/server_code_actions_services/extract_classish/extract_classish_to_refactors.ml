(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
let edit_of_candidate ~source_text:_ ~path _candidate : Lsp.WorkspaceEdit.t =
  let changes = SMap.singleton (Relative_path.to_absolute path) [] in
  Lsp.WorkspaceEdit.{ changes }

let to_refactor ~path ~source_text candidate : Code_action_types.Refactor.t =
  let edit = lazy (edit_of_candidate ~source_text ~path candidate) in
  Code_action_types.Refactor.{ title = "Extract interface"; edit }

let to_refactors ~source_text ~path candidate :
    Code_action_types.Refactor.t list =
  [to_refactor ~source_text ~path candidate]
