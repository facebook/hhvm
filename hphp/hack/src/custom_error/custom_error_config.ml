(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = Config of Custom_error.t list
[@@ocaml.unboxed] [@@deriving show, yojson]

let empty = Config []

let initialize file =
  try
    let (Config cs) = t_of_yojson @@ Yojson.Safe.from_file file in
    let (cs, errs) =
      List.partition_map
        (fun c ->
          match Custom_error.validate c with
          | (Validated.Valid c, _) -> Either.Left c
          | _ -> Either.Right c.Custom_error.name)
        cs
    in

    Ok (Config cs, errs)
  with
  | Yojson.Json_error err -> Error err
