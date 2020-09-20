(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let identity x = x

let rec funpow n ~f ~init =
  if n <= 0 then
    init
  else
    funpow (n - 1) ~f ~init:(f init)

let mk_combine keep_some combine env _ x y =
  match (x, y) with
  | (Some z, None)
  | (None, Some z) ->
    if keep_some then
      (env, Some z)
    else
      (env, None)
  | (Some x, Some y) ->
    let (env, z) = combine env x y in
    (env, Some z)
  | (None, None) -> (env, None)

let rec fold3 ~f ~init xs ys zs =
  List.Or_unequal_lengths.(
    match (xs, ys, zs) with
    | ([], [], []) -> Ok init
    | (x :: xs, y :: ys, z :: zs) ->
      begin
        match fold3 ~f ~init xs ys zs with
        | Ok acc -> Ok (f acc x y z)
        | err -> err
      end
    | _ -> Unequal_lengths)
