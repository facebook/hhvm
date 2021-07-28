(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Send status to persistent client, if any. Log in case of failure.
    Returns the timestamp of the send or None if nothing
    was sent, e.g. because of there is no persistent client
    or sending failed. *)
val send :
  ServerEnv.env -> ServerCommandTypes.busy_status -> ServerEnv.seconds option
