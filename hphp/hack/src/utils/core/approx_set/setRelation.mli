(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t =
  | Equal
  | Subset
  | Superset
  | Disjoint
  | Unknown

(** Changes a Subset relation to a Superset relation and vice versa *)
val flip : t -> t

(* Derived from Alloy model. See wellformed_set.als *)

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
