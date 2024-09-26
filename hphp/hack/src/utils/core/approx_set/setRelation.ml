(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
  sub: bool;
  super: bool;
  disjoint: bool;
}

let none = { sub = false; super = false; disjoint = false }

let subset = { none with sub = true }

let superset = { none with super = true }

let disjoint = { none with disjoint = true }

let equivalent = { none with sub = true; super = true }

let all = { sub = true; super = true; disjoint = true }

let make ~subset ~superset ~disjoint =
  { sub = subset; super = superset; disjoint }

let is_equivalent rel = rel.sub && rel.super

let is_subset rel = rel.sub

let is_superset rel = rel.super

let is_disjoint rel = rel.disjoint

let flip rel = { rel with sub = rel.super; super = rel.sub }

(* Derived from Alloy model. See wellformed_set.als *)

(** Let rel be the relation between the sets A and B.
    [complement] determines what a safe approximation of
    !A rel B, given we know A rel B
   *)
let complement rel = { sub = false; super = rel.disjoint; disjoint = rel.super }

(** Let rel be the relation between sets A and B.
    [union] determines what a safe approximation of
    (L ∪ R) rel A, given we know L rel A and R rel A
   *)
let union rel1 rel2 =
  {
    sub = rel1.sub && rel2.sub;
    super = rel1.super || rel2.super;
    disjoint = rel1.disjoint && rel2.disjoint;
  }

(** Let rel be the relation between sets A and B.
    [inter] determines what a safe approximation of
    (L ∩ R) rel A, given we know L rel A and R rel A
   *)
let inter rel1 rel2 =
  {
    sub = rel1.sub || rel2.sub;
    super = rel1.super && rel2.super;
    disjoint = rel1.disjoint || rel2.disjoint;
  }
