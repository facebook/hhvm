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
module T = Tast

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
             if tp.T.tp_reified = Tast.Erased then Some (snd tp.T.tp_name) else None)
    in
    List.for_all hl ~f:(function _, Aast.Happly ((_, id), []) ->
                          List.mem ~equal:String.equal erased_tparams id
                        | _ -> false)
  in
  let combine v1 v2 = match v1, v2 with
    | DefinitelyReified, _ | _, DefinitelyReified -> DefinitelyReified
    | MaybeReified, _ | _, MaybeReified -> MaybeReified
    | _ -> NotReified
  in
  match snd h with
  | Aast.Happly ((_, id), hl) ->
    if None <> Emit_expression.is_reified_tparam ~is_fun:true env id ||
       None <> Emit_expression.is_reified_tparam ~is_fun:false env id
    then DefinitelyReified else
      if List.is_empty hl || is_all_erased hl then NotReified else
      List.fold_right hl ~init:MaybeReified
        ~f:(fun h v -> combine v @@ has_reified_type_constraint env h)
  | Aast.Hsoft h
  | Aast.Hlike h
  | Aast.Hoption h -> has_reified_type_constraint env h
  | Aast.Hprim _
  | Aast.Hmixed
  | Aast.Hnonnull
  | Aast.Harray _
  | Aast.Hdarray _
  | Aast.Hvarray _
  | Aast.Hvarray_or_darray _
  | Aast.Hthis
  | Aast.Hnothing
  | Aast.Hdynamic
  | Aast.Htuple _
  | Aast.Hshape _
  | Aast.Hfun _
  | Aast.Haccess _ -> NotReified
  (* Not found in the original AST *)
  | Aast.Hany -> failwith "Should be a naming error"
  | Aast.Habstr _ ->
     failwith "TODO Unimplemented: Not in the original AST"

let rec remove_awaitable (pos, h_ as h) =
  match h_ with
  | Aast.Happly ((_, id), [h]) when String.lowercase id = "awaitable" -> h
  (* For @Awaitable<T>, the soft type hint is moved to the inner type, i.e @T *)
  | Aast.Hsoft h -> pos, Aast.Hsoft (remove_awaitable h)
  (* For ~Awaitable<T>, the like-type hint is moved to the inner type, i.e ~T *)
  | Aast.Hlike h -> pos, Aast.Hlike (remove_awaitable h)
  (* For ?Awaitable<T>, the optional is dropped *)
  | Aast.Hoption h -> remove_awaitable h
  | Aast.Htuple _
  | Aast.Hshape _
  | Aast.Hfun _
  | Aast.Haccess _
  | Aast.Happly _ -> h
  | Aast.Hany
  | Aast.Hmixed
  | Aast.Hnonnull
  | Aast.Habstr _
  | Aast.Harray _
  | Aast.Hdarray _
  | Aast.Hvarray _
  | Aast.Hvarray_or_darray _
  | Aast.Hprim _
  | Aast.Hthis
  | Aast.Hnothing
  | Aast.Hdynamic -> failwith "TODO Unimplemented Did not exist on legacy AST"

let convert_awaitable env h =
  if Ast_scope.Scope.is_in_async (Emit_env.get_scope env)
  then remove_awaitable h else h
