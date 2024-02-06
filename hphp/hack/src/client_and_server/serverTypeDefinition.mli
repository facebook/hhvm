(**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val go_quarantined :
  ctx:Provider_context.t ->
  entry:Provider_context.entry ->
  line:int ->
  column:int ->
  ServerCommandTypes.Go_to_type_definition.result
