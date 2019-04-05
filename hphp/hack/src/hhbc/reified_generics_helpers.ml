(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)
open Core_kernel
module A = Ast

type type_constraint =
  | DefinitelyReified (* There is a reified generic *)
  | MaybeReified      (* There is no function or class reified generics,
                       * but there may be an inferred one *)
  | NotReified
  | NoConstraint

let rec has_reified_type_constraint env h =
  let is_all_erased hl =
    let erased_tparams =
      Ast_scope.Scope.get_tparams (Emit_env.get_scope env)
      |> List.filter_map ~f:(fun tp ->
           if tp.A.tp_reified then None else Some (snd tp.A.tp_name))
    in
    List.for_all hl ~f:(function _, A.Happly ((_, id), []) ->
                          List.mem ~equal:String.equal erased_tparams id
                        | _ -> false)
  in
  let combine v1 v2 = match v1, v2 with
    | DefinitelyReified, _ | _, DefinitelyReified -> DefinitelyReified
    | MaybeReified, _ | _, MaybeReified -> MaybeReified
    | _ -> NotReified
  in
  match snd h with
  | A.Happly ((_, id), hl) ->
    if None <> Emit_expression.is_reified_tparam ~is_fun:true env id ||
       None <> Emit_expression.is_reified_tparam ~is_fun:false env id
    then DefinitelyReified else
      if List.is_empty hl || is_all_erased hl then NotReified else
        List.fold_right hl ~init:MaybeReified
          ~f:(fun h v -> combine v @@ has_reified_type_constraint env h)
  | A.Hsoft h
  | A.Hlike h
  | A.Hoption h -> has_reified_type_constraint env h
  | A.Htuple _
  | A.Hshape _
  | A.Hfun _
  | A.Haccess _ -> NotReified

let rec remove_awaitable (pos, _h as h) = match _h with
  | A.Happly ((_, id), [h]) when String.lowercase id = "awaitable" -> h
  (* For @Awaitable<T>, the soft type hint is moved to the inner type, i.e @T *)
  | A.Hsoft h -> pos, A.Hsoft (remove_awaitable h)
  (* For ~Awaitable<T>, the like-type hint is moved to the inner type, i.e ~T *)
  | A.Hlike h -> pos, A.Hlike (remove_awaitable h)
  (* For ?Awaitable<T>, the optional is dropped *)
  | A.Hoption h -> remove_awaitable h
  | A.Htuple _
  | A.Hshape _
  | A.Hfun _
  | A.Haccess _
  | A.Happly _ -> h

let convert_awaitable env h =
  if Ast_scope.Scope.is_in_async (Emit_env.get_scope env)
  then remove_awaitable h else h
