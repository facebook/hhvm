(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val go_quarantined :
  Provider_context.t ->
  Provider_context.entry ->
  File_content.Position.t ->
  string option
