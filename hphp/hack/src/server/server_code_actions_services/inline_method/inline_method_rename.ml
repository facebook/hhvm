(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

type t = {
  used_vars: String.Set.t;
  old_to_new: string String.Map.t;
}

let create ~used_vars = { used_vars; old_to_new = String.Map.empty }

let rename ({ used_vars; old_to_new } as t) old_var : t * string =
  let rec next_var (s : string) : string =
    if Set.mem used_vars s then
      next_var (s ^ "_")
    else
      s
  in
  match Map.find old_to_new old_var with
  | Some new_var -> (t, new_var)
  | None ->
    let new_var = next_var old_var in
    let t =
      {
        used_vars = Set.add used_vars new_var;
        old_to_new = Map.update old_to_new old_var ~f:(Fn.const new_var);
      }
    in
    (t, new_var)

let rename_all t old_vars =
  let fold old_var (t, new_vars) =
    let (t, new_var) = rename t old_var in
    (t, new_var :: new_vars)
  in
  List.fold_right old_vars ~init:(t, []) ~f:fold
