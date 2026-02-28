(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(**
- Dump classish positional information.
- Solely used for testing purposes in hh_single_type_check
*)
val dump :
  Provider_context.t -> Provider_context.entry -> Relative_path.t -> unit
