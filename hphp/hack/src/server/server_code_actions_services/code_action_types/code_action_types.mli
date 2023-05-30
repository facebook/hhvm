(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(** Internal representation of code actions for refactoring *)
module Refactor : sig
  type t = {
    title: string;
    edit: Lsp.WorkspaceEdit.t Lazy.t;
  }

  type find =
    entry:Provider_context.entry ->
    path:Relative_path.t ->
    range:Lsp.range ->
    Provider_context.t ->
    t list
end

(** Internal representation of code actions for quickfixes.
  * Note that we do not include diagnostics.
  * We can tell LSP which error this fixed, but we'd have to
  * recompute the diagnostic from the error and there's no clear benefit *)
module Quickfix : sig
  type t = {
    title: string;
    edit: Lsp.WorkspaceEdit.t Lazy.t;
  }
end
