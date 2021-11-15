(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(**
 * A Bloom Filter is a probablistic data structure representing a set.
 * Elements can be added to the set and supports membership queries.
 * Membership will never produce false negative (saying an element
 * is not a member of the set when it is), but does allow false positives
 * (saying an element is a member of the set when it is not). The false
 * positive rate will increase as members are added so it is important
 * to choose the initial capacity to match the number of elements you
 * expect to add
 **)
type t

type elt

(**
 * Creates a Bloom Filter of significant size to answer membership queries
 * with ~1% false positive rates if `capacity` or fewer elements are
 * added
 **)
val create : capacity:int -> t

(**
 * Transforms a string to a set of hashes, representing an potential element
 * of a Bloom Filter. The hashes are independent of the capacity of the
 * Bloom Filter, so the same `elt` can be used for multiple filters
 **)
val hash : string -> elt

val add : t -> elt -> unit

val mem : t -> elt -> bool
