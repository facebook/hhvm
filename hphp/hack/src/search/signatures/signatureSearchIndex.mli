(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type indexable_type =
 | ITprim of Nast.tprim
 | ITapply of string
 | ITdarray
 | ITvarray
 | ITvarray_or_darray
 | ITmixed
 | ITnonnull
 | ITdynamic

type search_term =
 | Arity of int
 | Parameter of {position: int; type_: indexable_type}
 | Return_type of indexable_type

type t

val make : unit -> t

val search_term_to_string : search_term -> string
(** Return a string representation of the given search term. This representation
    is guaranteed to be unique; only equivalent search_terms will be converted
    to the same string. *)

val fun_to_search_terms : string -> search_term list option
(** Look up the signature of the function with the given name and return the
    list of search_terms that correspond to that signature. If the function has
    a signature that we are not able to index, return None. *)

val update : t -> SSet.elt -> search_term list -> unit
(** Given a function name and a list of search terms to associate with that
    function, mutate the given index to associate those search terms with that
    function. *)

val get : t -> search_term -> SSet.t
(** Return the set of function names associated with the given search_term in
    the given index. *)
