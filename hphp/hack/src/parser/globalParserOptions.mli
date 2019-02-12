(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val get: unit -> ParserOptions.t
(** Get the global [ParserOptions.t].

    Raises [Failure] if [set] has not yet been invoked. *)

val set: ParserOptions.t -> unit
(** Set the global [ParserOptions.t] to be used in parsing for the entire
    lifetime of the server.

    Has no effect if [set] has already been invoked. *)
