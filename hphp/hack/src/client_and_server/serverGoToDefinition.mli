(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Returns the definition of the symbol at the given position in the document.

This function is for interactive use only, as it may return multiple definitions
for the user's convenience. For example, when hovering over a constructor call,
it may return both the definition for the class being constructed, and the
`__construct` method of the class. Tooling should use
[ServerCommandTypes.IDENTIFY_FUNCTION] instead. *)
val go_quarantined :
  ctx:Provider_context.t ->
  entry:Provider_context.entry ->
  line:int ->
  column:int ->
  ServerCommandTypes.Go_to_definition.result
