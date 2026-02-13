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
module DataType = Typing_data_type.DataType

let has_where_clauses variants =
  List.exists variants ~f:(fun v -> not @@ List.is_empty @@ snd v)

(** Given the variants of a case type (encoded as a locl_ty) and another locl_ty [intersecting_ty]
  produce a new locl_ty containing only the types in the variant that map to an intersecting
  data type. For example:
   Given

    [variants] = int | vec<int> | Vector<int>
    [intersecting_ty] = Container<string>

   This function will return the type `vec<int> | Vector<int>` because both `vec<int>` and
  `Vector<int>` overlap with the tag associated with `Container<string>`.

  Note that this function only considers the data type associated to each type and not
  the type itself. So even though `vec<int>` and `Container<string>` do not intersect at
  the type level, they do intersect when considering only the runtime data types. *)
let filter_variants_using_datatype
    ~safe_for_are_disjoint env reason variants intersecting_ty =
  let (env, tags) =
    DataType.fromTy ~safe_for_are_disjoint env intersecting_ty
  in
  let (env, vtags) =
    List.fold_map variants ~init:env ~f:(DataType.fromTy ~safe_for_are_disjoint)
  in
  let tyl =
    List.filter_map
      ~f:(fun (variant, variant_tags) ->
        if DataType.Set.are_disjoint env variant_tags tags then
          None
        else
          Some variant)
      (List.zip_exn variants vtags)
  in
  Typing_utils.union_list env reason tyl

(** Look up case type via [name].
  If the case type exists and all variants are unconditional, returns the list
  of variant types localized using [ty_args]
  If the case type doesn't exist or any variant has a where clause, returns
  [None].

  TODO T201569125 - Note: If we could use the ty_args to "evaluate" the where
  constraints, we could return the variant hints whose constraints are met, but
  I'm not sure if that's feasible.
*)
let get_variant_tys env name ty_args :
    Typing_env_types.env * locl_ty list option =
  match Env.get_typedef env name with
  | Decl_entry.Found
      { td_type_assignment = CaseType (variant, variants); td_tparams; _ } ->
    if has_where_clauses (variant :: variants) then
      (env, None)
    else
      let single_or_union =
        match (variant, variants) with
        | ((hint, _where_constraints), []) -> hint
        (* we're just going to take this union apart so the reason is irrelevant here *)
        | ((hint, _where_constraints), rest) ->
          Typing_make_type.union Reason.none @@ (hint :: List.map rest ~f:fst)
      in
      let ((env, _ty_err_opt), variants) =
        Typing_utils.localize_disjoint_union
          ~ety_env:
            {
              empty_expand_env with
              substs =
                (if List.is_empty ty_args then
                  SMap.empty
                else
                  Decl_subst.make_locl td_tparams ty_args);
            }
          env
          single_or_union
      in
      let tyl =
        match get_node variants with
        | Tunion tyl -> tyl
        | _ -> [variants]
      in
      (env, Some tyl)
  | _ -> (env, None)
