(*
 * Copyright (c) 2021, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Coverage_level
open Coverage_level_defs
open Reordered_argument_collections

let combine v1 v2 =
  SMap.merge ~f:(fun _ cs1 cs2 -> Option.merge cs1 cs2 ~f:merge_and_sum) v1 v2

class count_getter fixme_map =
  object
    inherit [level_stats SMap.t] Tast_visitor.reduce as super

    method zero = SMap.empty

    method plus = combine

    method! on_expr env expr =
      let acc = super#on_expr env expr in
      let ((pos, ty), e) = expr in
      let expr_kind_opt =
        match e with
        | Aast.Array_get _ -> Some "array_get"
        | Aast.Call _ -> Some "call"
        | Aast.Class_get _ -> Some "class_get"
        | Aast.Class_const _ -> Some "class_const"
        | Aast.Lvar _ -> Some "lvar"
        | Aast.New _ -> Some "new"
        | Aast.Obj_get _ -> Some "obj_get"
        | _ -> None
      in
      match expr_kind_opt with
      | None -> acc
      | Some kind ->
        let r = Typing_defs.get_reason ty in
        let (_env, lvl) = level_of_type env fixme_map (pos, ty) in
        let counter =
          match SMap.find_opt acc kind with
          | Some counter -> counter
          | None -> empty_counter
        in
        let ctx = Tast_env.get_ctx env in
        SMap.add acc ~key:kind ~data:(incr_counter ctx lvl (r, pos, counter))
  end

(* This should likely take in tasts made with type checker options that were
 * made permissive using TypecheckerOptions.make_permissive
 *)
let accumulate_types ctx tast check =
  let fixmes =
    Fixme_provider.get_hh_fixmes check |> Option.value ~default:IMap.empty
  in
  let cg = new count_getter fixmes in
  cg#go ctx tast
