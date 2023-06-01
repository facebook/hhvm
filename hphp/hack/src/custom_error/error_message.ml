(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type elem =
  | Lit of string
  | Ty_var of Patt_var.t
  | Name_var of Patt_var.t
[@@deriving show, yojson]

type t = { message: elem list } [@@deriving show, yojson]

let validate_elem elem ~env =
  match elem with
  | Lit _ -> Validated.valid elem
  | Ty_var var ->
    (match Validation_env.get env var with
    | Some Patt_binding_ty.Ty -> Validated.valid elem
    | _ -> Validated.invalid elem)
  | Name_var var ->
    (match Validation_env.get env var with
    | Some Patt_binding_ty.Name -> Validated.valid elem
    | _ -> Validated.invalid elem)

let validate ?(env = Validation_env.empty) { message } =
  ( Validated.map ~f:(fun message -> { message })
    @@ Validated.all
    @@ List.map (validate_elem ~env) message,
    env )
