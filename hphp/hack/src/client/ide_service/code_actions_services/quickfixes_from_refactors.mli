(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(**
 * Find quickfixes for errors where the quickfix comes from
 * repurposing a refactor.
 *
 * Note that quickfixes can come from several different places:
 * - Here. Best for when a quickfix is also useful as a refactor
 * or edits are expensive to compute
 * - From the full_fidelity parser implementation in Rust
 * - From errors generated from OCaml code
*)
val find :
  Provider_context.t ->
  Provider_context.entry ->
  Errors.error ->
  Code_action_types.Quickfix.t list
