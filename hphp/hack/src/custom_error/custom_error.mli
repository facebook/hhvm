(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
  name: string;
  patt: Patt_error.t;
  error_message: Error_message.t;
}
[@@deriving show, yojson]

include Can_validate.S with type t := t
