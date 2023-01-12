(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
module Inline_record_error : sig
  type t =
    | Inline_record of {
        a: int;
        b: t option;
      } [@transform.explicit]
    | Other of t option
  [@@deriving transform]
end = struct
  type t =
    | Inline_record of {
        a: int;
        b: t option;
      } [@transform.explicit]
    | Other of t option
  [@@deriving transform]
end
