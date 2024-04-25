(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** This module exposes constructors for additional commands implemented in
the nuclide.hack language extension *)

(** Command to select the given range in the editor. *)
val set_selection : Lsp.range -> Lsp.Command.t
