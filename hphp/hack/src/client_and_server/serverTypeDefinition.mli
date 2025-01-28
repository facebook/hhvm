(**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** To do with "go to type definition" functionality *)
val go_quarantined :
  ctx:Provider_context.t ->
  entry:Provider_context.entry ->
  File_content.Position.t ->
  ServerCommandTypes.Go_to_type_definition.result
