(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val eval :
  Tast_env.t ->
  Tast.expr ->
  (string, Typing_error.Primary.SimpliHack.t) Result.t
