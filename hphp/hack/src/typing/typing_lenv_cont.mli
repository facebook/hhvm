(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*****************************************************************************)
(* Functions dealing with continuation based flow typing of local variables *)
(*****************************************************************************)

include Typing_env_types_sig.S

val get_cont_option :
  Typing_continuations.t -> local_types -> local Local_id.Map.t option
val get_cont : Typing_continuations.t -> local_types -> local Local_id.Map.t
val add_to_cont :
  Typing_continuations.t -> Local_id.t -> local -> local_types -> local_types
val remove_from_cont :
  Typing_continuations.t -> Local_id.t -> local_types -> local_types
val drop_cont : Typing_continuations.t -> local_types -> local_types
val replace_cont :
  Typing_continuations.t -> local Local_id.Map.t -> local_types -> local_types
