(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** The result of handling a command message from a client. *)
type 'env handle_command_result =
  | Done of 'env
      (** The command was fully executed, and this is the new environment. *)
  | Needs_full_recheck of {
      env: 'env;
      finish_command_handling: 'env -> 'env;
          (** This continuation needs to be run after finished
              full check to complete the handling of the command. *)
      reason: string;
          (** Reason why this command needs full recheck (for logging/debugging purposes) *)
    }
      (** The command needs a full recheck to complete before it can finish being executed. *)

(** Wrap all the continuations inside result in provided try function *)
val wrap :
  try_:('env -> (unit -> 'env) -> 'env) ->
  'env handle_command_result ->
  'env handle_command_result

(** Shutdown given sockets. *)
val shutdown_client : 'in_channel * out_channel -> unit

val log_and_get_sharedmem_load_telemetry : unit -> Telemetry.t

(** Exit with exit code corresponding to given exception.
    Perform any necessary cleanups. *)
val exit_on_exception : Exception.t -> 'result

(** Execute given function. If function raises and exception,
    exit with exit code corresponding to given exception.
    Perform any necessary cleanups. *)
val with_exit_on_exception : (unit -> 'result) -> 'result

(** Return all the files that we need to typecheck *)
val make_next :
  ?hhi_filter:(string -> bool) ->
  indexer:(unit -> string list) ->
  extra_roots:Path.t list ->
  Relative_path.t list Bucket.next

(** If the components of this if statement look like they were
    desugared from an invariant() call, return the equivalent
    invariant() expression. *)
val resugar_invariant_call :
  Tast_env.env -> Tast.expr -> Tast.block -> Tast.expr option
