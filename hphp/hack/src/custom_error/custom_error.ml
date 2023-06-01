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

let validate ?(env = Validation_env.empty) t =
  let (patt, env) = Patt_error.validate t.patt ~env in
  let (error_message, _) = Error_message.validate t.error_message ~env in
  ( Validated.map2
      ~f:(fun patt error_message -> { t with patt; error_message })
      patt
      error_message,
    env )
