(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
module X = struct
  type t = {
    actual_ty_string: string;
    actual_ty_json: string;
    expected_ty_string: string;
    expected_ty_json: string;
    pos: Pos.t;
  }
  [@@deriving ord]
end

include X
module Set = Set.Make (X)

type result = t list
