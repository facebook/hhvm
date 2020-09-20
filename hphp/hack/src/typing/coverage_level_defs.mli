(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Reordered_argument_collections

module CLKey : sig
  type t = Ide_api_types.coverage_level

  val compare : t -> t -> int
end

module CLMap : sig
  include module type of WrappedMap.Make (CLKey)
end

type checked_stats = {
  unchecked: int;
  partial: int;
  checked: int;
}

(* result is an association list from absolute position in the code file to the
 * coverage level of the expression at that position, paired with a count of
 * the total instances of each coverage level.
 * Note in usage that the list does not contain every
 * instance, only those which do not themselves contain worse type-level
 * expressions as subexpressions.
 * This way when it is used to show typing information,
 * large chunks of code that are poorly typed do not obscure
 * the root cause of their poor typing, which would be some poorly
 * typed subexpression.
 *)
type result = (Pos.absolute * Ide_api_types.coverage_level) list * checked_stats

type pos_stats_entry = {
  (* How many times this reason position has occured. *)
  pos_count: int;
  (* Random sample of expressions where this reason position has occured, for
   * debugging purposes *)
  samples: Pos.t list;
}

type level_stats_entry = {
  (* Number of expressions of this level *)
  count: int;
  (* string of reason -> position of reason -> stats *)
  reason_stats: pos_stats_entry Pos.Map.t SMap.t;
}

(* An assoc list that counts the number of expressions at each coverage level,
 * along with stats about their reasons *)
type level_stats = level_stats_entry CLMap.t

(* There is a trie in utils/, but it is not quite what we need ... *)

type 'a trie =
  | Leaf of 'a
  | Node of 'a * 'a trie SMap.t
