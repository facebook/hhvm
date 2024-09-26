(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t

val make : subset:bool -> superset:bool -> disjoint:bool -> t

(** Assume no relations exists *)
val none : t

(** Assume a subset relationship exists *)
val subset : t

(** Assume a superset relationship exist *)
val superset : t

(** Assume a disjoint relationship exist *)
val disjoint : t

(** Assume the sets are equivalent. This means it is both
    a subset and a superset *)
val equivalent : t

(** Assume all relations exists (subset, superset & disjoint).
    This is only valid when relating the empty set to itself. *)
val all : t

(** Check if the relation implies equivalence. This is
    true iff [is_subset] and [is_superset] are true. *)
val is_equivalent : t -> bool

(** Check if the relation implies subset *)
val is_subset : t -> bool

(** Check if the relation implies superset *)
val is_superset : t -> bool

(** Check if the relation implies disjointness *)
val is_disjoint : t -> bool

(** Changes a Subset relation to a Superset relation and vice versa *)
val flip : t -> t

(** Let rel be the relation between the sets A and B.
    [complement] determines what a safe approximation of
    !A rel B, given we know A rel B
   *)
val complement : t -> t

(** Let rel be the relation between sets A and B.
    [union] determines what a safe approximation of
    (L ∪ R) rel A, given we know L rel A and R rel A
   *)
val union : t -> t -> t

(** Let rel be the relation between sets A and B.
    [inter] determines what a safe approximation of
    (L ∩ R) rel A, given we know L rel A and R rel A
   *)
val inter : t -> t -> t
