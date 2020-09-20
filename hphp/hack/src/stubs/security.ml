(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type error = string

type success = Checks_skipped [@@deriving show]

let check_credentials ~attempt_fix =
  ignore attempt_fix;
  Ok Checks_skipped

let to_error_message_string error = error

let to_error_kind_string error = error
