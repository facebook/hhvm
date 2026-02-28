(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
val add_typing_error : Typing_error.t -> env:Typing_env_types.env -> unit

val ambiguous_inheritance :
  Pos_or_decl.t ->
  string ->
  string ->
  (Pos.t, Pos_or_decl.t) User_diagnostic.t ->
  Typing_error.Reasons_callback.t ->
  env:Typing_env_types.env ->
  unit

(** Inspect the typing error, and update it by changing the primary position, or dropping the
    outermost wrapper. When checking a method call with subtyping, we use the whole
    call expression's position and add an "Illegal argument" message. If one of the parameters
    doesn't match, this focusses on the position. Alternately, if something else went wrong, we
    need to drop the "Invalid argument" (e.g., if the method isn't there). *)
val update_error_for_method_call :
  arg_posl:Pos.t list -> Typing_error.t -> Typing_error.t
