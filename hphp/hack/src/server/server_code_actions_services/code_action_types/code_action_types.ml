(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
module Refactor = struct
  type t = {
    title: string;
    edit: Lsp.WorkspaceEdit.t Lazy.t;
  }

  type find =
    entry:Provider_context.entry ->
    range:Lsp.range ->
    Provider_context.t ->
    t list
end

module Quickfix = struct
  type t = {
    title: string;
    edit: Lsp.WorkspaceEdit.t Lazy.t;
  }
end
