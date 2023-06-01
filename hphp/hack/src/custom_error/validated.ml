(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
[@@@warning "-66"]

module X = struct
  type 'a t =
    | Valid of 'a
    | Invalid of 'a
  [@@deriving compare, sexp]

  let valid x = Valid x

  let invalid x = Invalid x

  let return x = Valid x

  let map_ t ~f =
    match t with
    | Valid x -> Valid (f x)
    | Invalid x -> Invalid (f x)

  let map = `Custom map_

  let map2 t1 t2 ~f =
    match (t1, t2) with
    | (Valid x, Valid y) -> Valid (f x y)
    | (Valid x, Invalid y)
    | (Invalid x, Valid y)
    | (Invalid x, Invalid y) ->
      Invalid (f x y)
end

include X
include Core.Applicative.Make_using_map2 (X)
