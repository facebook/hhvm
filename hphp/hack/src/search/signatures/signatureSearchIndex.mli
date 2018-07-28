(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type query =
  | And of query list
  | Or of query list
  | Term of string

type t

val make : unit -> t

val update : t -> string -> string list -> unit
(** Given a function name and a list of search terms to associate with that
    function, mutate the given index to associate these search terms with the given
    function. *)

val get : t -> query -> string list
(** Return an ordered list of function names associated with the given query
    from the index *)
