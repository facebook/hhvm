(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
module Constant_ctor_error : sig
  type t =
    | Constant [@deriving transform.explicit]
    | Other of t option
  [@@deriving transform]
end = struct
  type t =
    | Constant [@deriving transform.explicit]
    | Other of t option
  [@@deriving transform]
end
