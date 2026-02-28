(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
val parse :
  string ->
  Full_fidelity_positioned_syntax.t * Full_fidelity_syntax_error.t list

val hackfmt : string -> string
