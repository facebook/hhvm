(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

exception Nonfatal_rpc_exception of Exception.t * ServerEnv.env

(** The priority pipe is only for "read-only" commands.
Invariant (checked in [ServerMain.priority_client_interrupt_handler]):
if a command uses the priority pipe, then [handle] will only
ever return [Done]. *)
val use_priority_pipe : 'result ServerCommandTypes.t -> bool

(** Handle a client command. This can either execute the command immediately,
or store it as a continuation to be completed later
(when full recheck is completed, when workers are available,
when current recheck is cancelled...).
Invariant (checked in [ServerMain.priority_client_interrupt_handler]):
if this returns anything other than [Done], then [use_priority_pipe]
must have returned false for the command we fetch from the client to handle. *)
val handle :
  ServerEnv.genv ->
  ServerEnv.env ->
  ClientProvider.client ->
  ServerEnv.env ServerUtils.handle_command_result
