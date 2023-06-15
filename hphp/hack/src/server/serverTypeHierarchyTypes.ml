(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type memberKind =
  | Method [@value 1]
  | SMethod [@value 2]
  | Property [@value 3]
  | SProperty [@value 4]
  | Const [@value 5]
[@@deriving show]

type memberEntry = {
  name: string;
  snippet: string;
  kind: memberKind;
  pos: Pos.absolute;
  origin: string;
}

type entryKind =
  | Class [@value 1]
  | Interface [@value 2]
  | Enum [@value 3]
  | Trait [@value 4]
[@@deriving show]

type ancestorEntry =
  | AncestorName of string
  | AncestorDetails of {
      name: string;
      kind: entryKind;
      pos: Pos.absolute;
    }

type hierarchyEntry = {
  name: string;
  kind: entryKind;
  pos: Pos.absolute;
  ancestors: ancestorEntry list;
  members: memberEntry list;
}

type result = hierarchyEntry option
