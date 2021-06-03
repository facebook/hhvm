(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs
module Env = Typing_env
module TDef = Typing_tdef
module N = Aast

(*****************************************************************************)
(* Check if a comparison is trivially true or false *)
(*****************************************************************************)

let trivial_result_str bop =
  match bop with
  | Ast_defs.Eqeqeq -> Markdown_lite.md_codify "false"
  | Ast_defs.Diff2 -> Markdown_lite.md_codify "true"
  | _ -> assert false

let trivial_comparison_error env p bop ty1 ty2 trail1 trail2 =
  let trivial_result = trivial_result_str bop in
  let tys1 = Typing_print.error env ty1 in
  let tys2 = Typing_print.error env ty2 in
  Errors.trivial_strict_eq
    p
    trivial_result
    (Reason.to_string ("This is " ^ tys1) (get_reason ty1))
    (Reason.to_string ("This is " ^ tys2) (get_reason ty2))
    trail1
    trail2

let eq_incompatible_types env p ty1 ty2 =
  let tys1 = Typing_print.error env ty1 in
  let tys2 = Typing_print.error env ty2 in
  Errors.eq_incompatible_types
    p
    (Reason.to_string ("This is " ^ tys1) (get_reason ty1))
    (Reason.to_string ("This is " ^ tys2) (get_reason ty2))

let is_arraykey t =
  match get_node t with
  | Tprim N.Tarraykey -> true
  | _ -> false

let bad_compare_prim_to_enum ty enum_bound =
  match get_node enum_bound with
  | Tprim enum_bound ->
    (match (enum_bound, ty) with
    | (N.Tint, (N.Tint | N.Tarraykey | N.Tnum)) -> false
    | (N.Tint, _) -> true
    | (N.Tstring, (N.Tstring | N.Tarraykey)) -> false
    | (N.Tstring, _) -> true
    | (N.Tarraykey, (N.Tarraykey | N.Tint | N.Tstring | N.Tnum)) -> false
    | (N.Tarraykey, _) -> true
    | _ -> false)
  | _ -> false

let rec assert_nontrivial p bop env ty1 ty2 =
  let ety_env = empty_expand_env in
  let (_, ty1) = Env.expand_type env ty1 in
  let (_, ety1, trail1) = TDef.force_expand_typedef ~ety_env env ty1 in
  let (_, ty2) = Env.expand_type env ty2 in
  let (_, ety2, trail2) = TDef.force_expand_typedef ~ety_env env ty2 in
  match (get_node ty1, get_node ty2) with
  (* Disallow `===` on distinct abstract enum types. *)
  (* Future: consider putting this in typed lint not type checking *)
  | (Tnewtype (e1, _, bound1), Tnewtype (e2, _, bound2))
    when Env.is_enum env e1
         && Env.is_enum env e2
         && is_arraykey bound1
         && is_arraykey bound2 ->
    if String.equal e1 e2 then
      ()
    else
      eq_incompatible_types env p ety1 ety2
  | _ ->
    (match (deref ety1, deref ety2) with
    | ((_, Tprim N.Tnum), (_, Tprim (N.Tint | N.Tfloat)))
    | ((_, Tprim (N.Tint | N.Tfloat)), (_, Tprim N.Tnum)) ->
      ()
    | ((_, Tprim N.Tarraykey), (_, Tprim (N.Tint | N.Tstring)))
    | ((_, Tprim (N.Tint | N.Tstring)), (_, Tprim N.Tarraykey)) ->
      ()
    | ((_, Tprim N.Tnull), _)
    | (_, (_, Tprim N.Tnull)) ->
      ()
    | ((r, Tprim N.Tnoreturn), _)
    | (_, (r, Tprim N.Tnoreturn)) ->
      Errors.noreturn_usage p (Reason.to_string "This always throws or exits" r)
    | ((r, Tprim N.Tvoid), _)
    | (_, (r, Tprim N.Tvoid)) ->
      (* Ideally we shouldn't hit this case, but well... *)
      Errors.void_usage p (Reason.to_string "This is `void`" r)
    | ((_, Tprim a), (_, Tnewtype (e, _, bound)))
      when Env.is_enum env e && bad_compare_prim_to_enum a bound ->
      trivial_comparison_error env p bop ty1 bound trail1 trail2
    | ((_, Tnewtype (e, _, bound)), (_, Tprim a))
      when Env.is_enum env e && bad_compare_prim_to_enum a bound ->
      trivial_comparison_error env p bop bound ty2 trail1 trail2
    | ((_, Tprim a), (_, Tprim b)) when not (Aast.equal_tprim a b) ->
      trivial_comparison_error env p bop ty1 ty2 trail1 trail2
    | ((_, Toption ty1), (_, Tprim _)) -> assert_nontrivial p bop env ty1 ty2
    | ((_, Tprim _), (_, Toption ty2)) -> assert_nontrivial p bop env ty1 ty2
    | ( ( _,
          ( Terr | Tany _ | Tnonnull | Tvarray _ | Tdarray _
          | Tvarray_or_darray _ | Tvec_or_dict _ | Tprim _ | Toption _
          | Tdynamic | Tvar _ | Tfun _ | Tgeneric _ | Tnewtype _ | Tdependent _
          | Tclass _ | Ttuple _ | Tunion _ | Tintersection _ | Tobject
          | Tshape _ | Taccess _ | Tunapplied_alias _ | Tneg _ ) ),
        _ ) ->
      ())

let assert_nullable p bop env ty =
  let (_, ty) = Env.expand_type env ty in
  match deref ty with
  | (r, (Tvarray _ | Tdarray _ | Tvarray_or_darray _)) ->
    let trivial_result = trivial_result_str bop in
    let ty_str = Typing_print.error env ty in
    let msgl =
      Reason.to_string ("This is " ^ ty_str ^ " and cannot be null") r
    in
    Errors.trivial_strict_not_nullable_compare_null p trivial_result msgl
  | (_, _) -> ()
