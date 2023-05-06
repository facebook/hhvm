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
  | Ast_defs.Eqeqeq -> false
  | Ast_defs.Diff2 -> true
  | _ -> assert false

let trivial_comparison_error env p bop ty1 ty2 trail1 trail2 =
  let result = trivial_result_str bop
  and tys1 = lazy (Typing_print.error env ty1)
  and tys2 = lazy (Typing_print.error env ty2) in
  Typing_error_utils.add_typing_error
    Typing_error.(
      primary
      @@ Primary.Trivial_strict_eq
           {
             pos = p;
             result;
             left =
               Lazy.map tys1 ~f:(fun tys1 ->
                   Reason.to_string ("This is " ^ tys1) (get_reason ty1));
             right =
               Lazy.map tys2 ~f:(fun tys2 ->
                   Reason.to_string ("This is " ^ tys2) (get_reason ty2));
             left_trail = trail1;
             right_trail = trail2;
           })

let eq_incompatible_types env p ty1 ty2 =
  let tys1 = lazy (Typing_print.error env ty1)
  and tys2 = lazy (Typing_print.error env ty2) in
  Typing_error_utils.add_typing_error
    Typing_error.(
      primary
      @@ Primary.Eq_incompatible_types
           {
             pos = p;
             left =
               Lazy.map tys1 ~f:(fun tys1 ->
                   Reason.to_string ("This is " ^ tys1) (get_reason ty1));
             right =
               Lazy.map tys2 ~f:(fun tys2 ->
                   Reason.to_string ("This is " ^ tys2) (get_reason ty2));
           })

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
    | ((_, Tprim N.Tnum), (_, Tprim N.Tarraykey))
    | ((_, Tprim N.Tarraykey), (_, Tprim N.Tnum)) ->
      ()
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
      Typing_error_utils.add_typing_error
        Typing_error.(
          wellformedness
          @@ Primary.Wellformedness.Noreturn_usage
               {
                 pos = p;
                 reason =
                   lazy (Reason.to_string "This always throws or exits" r);
               })
    | ((r, Tprim N.Tvoid), _)
    | (_, (r, Tprim N.Tvoid)) ->
      (* Ideally we shouldn't hit this case, but well... *)
      Typing_error_utils.add_typing_error
        Typing_error.(
          wellformedness
          @@ Primary.Wellformedness.Void_usage
               { pos = p; reason = lazy (Reason.to_string "This is `void`" r) })
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
          ( Tany _ | Tnonnull | Tvec_or_dict _ | Tprim _ | Toption _ | Tdynamic
          | Tvar _ | Tfun _ | Tgeneric _ | Tnewtype _ | Tdependent _ | Tclass _
          | Ttuple _ | Tunion _ | Tintersection _ | Tshape _ | Taccess _
          | Tunapplied_alias _ | Tneg _ ) ),
        _ ) ->
      ())
