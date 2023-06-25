(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type versioned_patt_error = Error_v1 of Patt_error.t
[@@deriving eq, show] [@@boxed]

type versioned_error_message = Message_v1 of Error_message.t
[@@deriving eq, show] [@@boxed]

type t = {
  name: string;
  patt: versioned_patt_error;
  error_message: versioned_error_message;
}
[@@deriving eq, show]
