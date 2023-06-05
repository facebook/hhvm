(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = { custom_errors: Custom_error.t list } [@@deriving eq, show, yojson]

let empty = { custom_errors = [] }

let initialize file =
  try
    let { custom_errors } = t_of_yojson @@ Yojson.Safe.from_file file in
    let (custom_errors, errs) =
      List.partition_map
        (fun c ->
          match Custom_error.validate c with
          | (Validated.Valid c, _) -> Either.Left c
          | _ -> Either.Right c.Custom_error.name)
        custom_errors
    in

    Ok ({ custom_errors }, errs)
  with
  | Yojson.Json_error err -> Error err
