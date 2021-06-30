(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

exception Nonfatal_rpc_exception of Exception.t * ServerEnv.env

(** Some client commands require full check to be run in order to update global
    state that they depend on *)
val rpc_command_needs_full_check : 'result ServerCommandTypes.t -> bool

val rpc_command_needs_writes : 'result ServerCommandTypes.t -> bool

(** Handle a client command. This can either execute the command immediately,
    or store it as a continuation to be completed later
    (when full recheck is completed, when workers are available,
    when current recheck is cancelled...) *)
val handle :
  ServerEnv.genv ->
  ServerEnv.env ->
  ClientProvider.client ->
  ServerEnv.env ServerUtils.handle_command_result
