(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type t = {
  actual_ty_string: string;
  actual_ty_json: string;
  expected_ty_string: string;
  expected_ty_json: string;
  pos: Pos.t;
}

type result = t list

val compare : t -> t -> int

module Set : Set.S with type elt := t
