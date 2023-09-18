(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

type 'a t = {
  under_normal_assumptions: 'a;
  under_dynamic_assumptions: 'a option;
}
[@@deriving eq, hash, show]

let mk_without_dynamic under_normal_assumptions =
  { under_normal_assumptions; under_dynamic_assumptions = None }

let map ~f x =
  {
    under_normal_assumptions = f x.under_normal_assumptions;
    under_dynamic_assumptions = Option.map ~f x.under_dynamic_assumptions;
  }

let combine ~f x y =
  {
    under_normal_assumptions =
      f x.under_normal_assumptions y.under_normal_assumptions;
    under_dynamic_assumptions =
      begin
        match (x.under_dynamic_assumptions, y.under_dynamic_assumptions) with
        | (None, None) -> None
        | (Some x, None) -> Some x
        | (None, Some y) -> Some y
        | (Some x, Some y) -> Some (f x y)
      end;
  }

let all x =
  let { under_normal_assumptions; under_dynamic_assumptions } = x in
  match under_dynamic_assumptions with
  | None -> [under_normal_assumptions]
  | Some under_dynamic_assumptions ->
    [under_normal_assumptions; under_dynamic_assumptions]

let append xs ys = combine ~f:( @ ) xs ys

let cons x xs = append (map ~f:(fun x -> x :: []) x) xs

let collect xs = List.fold_right ~f:cons ~init:(mk_without_dynamic []) xs
