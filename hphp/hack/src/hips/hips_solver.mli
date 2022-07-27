(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hips_types

(*
 * One step substitution
 * For example:
 * substitute
 * (Arg f 0 p)
 * {f: [Intra Has_static_key(f0, 'a', int), Inter (Arg g 0 f1)]}
 * =
 * [Intra Has_static_key(p, 'a', int), Inter (Arg g 0 f1)]
 *)
val substitute :
  'entity inter_constraint ->
  ('intra_constraint, 'entity) constraint_ list SMap.t ->
  ('intra_constraint, 'entity) constraint_ list

(*
 * Full analysis
 * Removes interprocedural constraints using multiple one step substitutions
 *)
val analyse :
  ('intra_constraint, 'entity) constraint_ list SMap.t -> 'intra_constraint list
