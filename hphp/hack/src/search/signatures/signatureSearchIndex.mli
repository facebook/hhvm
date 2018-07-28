(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t

val make : unit -> t

val update : t -> string -> string list -> unit
(** Given a function name and a list of search terms to associate with that
    function, mutate the given index to associate those search terms with the given
    function. *)

val get : t -> string -> SSet.t
(** Return the set of function names associated with the given search_term in
    the given index. *)
