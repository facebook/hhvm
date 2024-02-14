(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(**
 * Error messages for which the best quickfix is to repurpose
 * a refactor.
 *
 * Note that quickfixes can come from several different places:
 * - From the full_fidelity parser implementation in Rust
 * - From errors generated from OCaml code
 * - And here.
*)
val mapping_from_error_message_to_refactors :
  Code_action_types.Refactor.find SMap.t
