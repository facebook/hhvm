(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** [Error] on a combination of [--isolation-*] flags the run cannot honour.
    Combinations it can honour but that almost certainly do not mean what the
    caller intended are warned about on stderr, since they do not stop the run.

    [max_dependents_given] tells the two apart for that bound: it reaches here
    resolved to its default, and warning about a value the caller never chose
    would report their own default back at them. *)
val validate :
  max_dependents_given:bool ->
  Server_isolation_types.options ->
  (unit, string) Stdlib.result
