(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val find_candidate :
  selection:Pos.t ->
  entry:Provider_context.entry ->
  Provider_context.t ->
  Extract_method_types.candidate option
