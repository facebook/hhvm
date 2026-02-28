(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(**
- Enables previewing IDE diagnostic indicators from the command line
- Currently used only in hh_single_type_check
*)
val run : Provider_context.t -> Provider_context.entry -> Diagnostics.t -> unit
