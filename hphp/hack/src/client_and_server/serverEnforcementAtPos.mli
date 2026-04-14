(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val go_ctx :
  ctx:Provider_context.t ->
  entry:Provider_context.entry ->
  File_content.Position.t ->
  EnforcementAtPosService.result

val result_to_json_string :
  EnforcementAtPosService.result -> string * File_content.Position.t -> string
