(* Copyright (c) 2020, Facebook, Inc.
   All rights reserved. *)

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
