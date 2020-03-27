(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Reordered_argument_collections

module CLKey = struct
  type t = Ide_api_types.coverage_level

  let compare x y = Ide_api_types.compare_coverage_level x y
end

module CLMap = struct
  include WrappedMap.Make (CLKey)
end

type checked_stats = {
  unchecked: int;
  partial: int;
  checked: int;
}

type result = (Pos.absolute * Ide_api_types.coverage_level) list * checked_stats

type pos_stats_entry = {
  pos_count: int;
  samples: Pos.t list;
}

type level_stats_entry = {
  count: int;
  reason_stats: pos_stats_entry Pos.Map.t SMap.t;
}

type level_stats = level_stats_entry CLMap.t

type 'a trie =
  | Leaf of 'a
  | Node of 'a * 'a trie SMap.t
