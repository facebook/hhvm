(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type entryKind =
  | Class [@value 1]
  | Interface [@value 2]
  | Enum [@value 3]
  | Trait [@value 4]
[@@deriving show]

type hierarchyEntry = {
  name: string;
  kind: entryKind;
  pos: Pos.t;
  ancestors: string list;
}

type result = hierarchyEntry option
