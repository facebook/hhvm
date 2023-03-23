(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
val program : Provider_context.t -> Tast.program -> unit

val def : Provider_context.t -> Tast.def -> unit
