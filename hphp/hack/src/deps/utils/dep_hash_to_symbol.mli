(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val from_nasts :
  Nast.program list ->
  Typing_deps.Dep.dependency Typing_deps.Dep.variant Typing_deps.DepMap.t

val dump : Nast.program -> unit
