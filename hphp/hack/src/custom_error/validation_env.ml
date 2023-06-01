(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
[@@@warning "-66"]

open Core

type t = Env of Patt_binding_ty.t Core.String.Map.t [@@ocaml.unboxed]

let empty = Env String.Map.empty

let add (Env t) ~key ~data =
  match Core.String.Map.add ~key ~data t with
  | `Duplicate -> Error (Env t)
  | `Ok t -> Ok (Env t)

let get (Env t) key = Core.String.Map.find t key

let errors t1 t2 =
  Core.List.partition_map ~f:snd
  @@ Core.Map.to_alist
  @@ Core.Map.merge
       ~f:(fun ~key:lbl elt ->
         let open Patt_binding_ty in
         match elt with
         | `Both (Name, Name) -> None
         | `Both (Ty, Ty) -> None
         | `Both (a, b) -> Some (Either.Second (Validation_err.Mismatch (a, b)))
         | `Left _ -> Some (Either.Second (Validation_err.Unbound lbl))
         | `Right _ -> Some (Either.First (Validation_err.Unbound lbl)))
       t1
       t2

let merge_left t1 t2 =
  Core.Map.merge
    ~f:(fun ~key:_ elt ->
      match elt with
      | `Both (l, _)
      | `Left l ->
        Some l
      | `Right r -> Some r)
    t1
    t2

let combine (Env t1) (Env t2) =
  let t = merge_left t1 t2 in
  (Env t, errors t1 t2)
