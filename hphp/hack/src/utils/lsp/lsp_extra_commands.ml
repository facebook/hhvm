(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
let set_selection (range : Lsp.range) : Lsp.Command.t =
  Lsp.Command.
    {
      title = "Set Cursor Selection";
      command = "hack.setSelection";
      arguments = [Lsp_fmt.print_range range];
    }
