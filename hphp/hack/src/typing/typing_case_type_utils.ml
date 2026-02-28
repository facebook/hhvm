(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs
module Env = Typing_env

let find_recursive_mentions_in_decl_ty_via_typedef env name ty =
  let rec visit seen ty =
    let visitor =
      object (_this)
        inherit [decl_ty list] Type_visitor.decl_type_visitor as super

        method! on_tapply acc reason id args =
          let (_pos, sid) = id in
          let mentions =
            if String.equal sid name then
              [Typing_defs.mk (reason, Tapply (id, args))]
            else if SSet.mem sid seen then
              []
            else
              let seen = SSet.add sid seen in
              match Env.get_class_or_typedef env sid with
              | Decl_entry.Found (Env.TypedefResult typedef_info) -> begin
                match typedef_info.td_type_assignment with
                | SimpleTypeDef (_, hint) -> visit seen hint
                | CaseType (variant, variants) ->
                  let hints =
                    List.concat_map (variant :: variants) ~f:(fun (hint, wcs) ->
                        hint
                        :: List.concat_map wcs ~f:(fun (h1, _, h2) -> [h1; h2]))
                  in
                  List.concat_map hints ~f:(visit seen)
              end
              | _ -> []
          in
          mentions @ super#on_tapply acc reason id args
      end
    in
    visitor#on_type [] ty
  in
  visit SSet.empty ty

let decl_ty_mentions_name_via_typedef env name ty =
  not
  @@ List.is_empty
  @@ find_recursive_mentions_in_decl_ty_via_typedef env name ty

let find_where_clause_recursive_mentions
    env t_name (where_constraints : Aast.where_constraint_hint list) =
  let find_recursive_mentions hint =
    let ty = Decl_hint.hint env.Typing_env_types.decl_env hint in
    let (_pos, name) = t_name in
    find_recursive_mentions_in_decl_ty_via_typedef env name ty
  in
  List.map where_constraints ~f:(fun ((h1, _, h2) as wc) ->
      let mentions = find_recursive_mentions h1 @ find_recursive_mentions h2 in
      (wc, mentions))

let filter_where_clauses_with_recursive_mentions env t_name where_constraints =
  find_where_clause_recursive_mentions env t_name where_constraints
  |> List.filter_map ~f:(fun (wc, mentions) ->
         if List.is_empty mentions then
           Some wc
         else
           None)
