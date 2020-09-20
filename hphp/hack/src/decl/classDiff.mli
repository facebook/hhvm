(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type member_change =
  | Added
  | Removed
  | Changed_inheritance (* Modified in a way that affects inheritance *)
  | Modified (* Modified in a way that does not affect inheritance *)
  | Private_change (* Added/removed a private member *)
[@@deriving eq, show]

type member_diff = {
  consts: member_change SMap.t;
  typeconsts: member_change SMap.t;
  props: member_change SMap.t;
  sprops: member_change SMap.t;
  methods: member_change SMap.t;
  smethods: member_change SMap.t;
  constructor: member_change option;
}
[@@deriving eq, show]

type minor_change = {
  mro_positions_changed: bool;
  member_diff: member_diff;
}
[@@deriving eq, show]

type t =
  | Unchanged
  | Major_change
  | Minor_change of minor_change
[@@deriving eq, show]

val empty_member_diff : member_diff

val is_empty_member_diff : member_diff -> bool

val change_affects_descendants : member_change -> bool
