(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* These values are stored in the naming table database, so be careful when
changing them! *)
type kind_of_type =
  | TClass [@value 0]
  | TTypedef [@value 1]
  | TRecordDef [@value 2]
[@@deriving eq, enum]
