(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val program : Provider_context.t -> Nast.program -> unit

val def : Provider_context.t -> Nast.def -> unit
