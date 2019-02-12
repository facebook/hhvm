(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val get: unit -> TypecheckerOptions.t
(** Get the global [TypecheckerOptions.t] to be used in Naming and Decl.

    Raises [Failure] if [set] has not yet been invoked. *)

val set: TypecheckerOptions.t -> unit
(** Set the global [TypecheckerOptions.t] to be used in Naming and Decl for the
    entire lifetime of the server.

    Has no effect if [set] has already been invoked. *)
