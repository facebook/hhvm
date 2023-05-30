(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
let lsp_range_of_ide_range (ide_range : Ide_api_types.range) : Lsp.range =
  let module I = Ide_api_types in
  let lsp_pos_of_ide_pos ide_pos =
    Lsp.{ line = ide_pos.I.line; character = ide_pos.I.column }
  in
  Lsp.
    {
      start = lsp_pos_of_ide_pos ide_range.I.st;
      end_ = lsp_pos_of_ide_pos ide_range.I.ed;
    }

let find
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(path : Relative_path.t)
    ~(range : Ide_api_types.range) : Code_action_types.Refactor.t list =
  let lsp_range = lsp_range_of_ide_range range in
  let variable_actions =
    match Inline_variable.find ~range:lsp_range ~path ~entry ctx with
    | [] -> Extract_variable.find ~range:lsp_range ~path ~entry ctx
    | actions -> actions
  in
  Override_method.find ~range:lsp_range ~path ~entry ctx
  @ variable_actions
  @ Extract_method.find ~range:lsp_range ~path ~entry ctx
  @ Flip_around_comma.find ~range:lsp_range ~path ~entry ctx
