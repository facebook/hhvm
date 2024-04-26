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

(** Parse a [set_selection] command into the given range.

 - Returns [None] if the given command wasn't a [set_selection] command.
 - Returns [Error] if the command name matched, but the arguments of the
   command could not be parsed. *)
val parse_set_selection : Lsp.Command.t -> (Lsp.range option, string) Result.t
