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

let trivial_comparison_error
    env pos bop ty1 ty2 left_trail right_trail ~as_warning =
  let result = trivial_result_str bop in
  let describe_type ty =
    let ty_string = lazy (Typing_print.error env ty) in
    Lazy.map ty_string ~f:(fun ty_string ->
        Reason.to_string ("This is " ^ ty_string) (get_reason ty))
  in
  let left = describe_type ty1 in
  let right = describe_type ty2 in
  if as_warning then
    Typing_warning_utils.add
      env
      ( pos,
        Typing_warning.Sketchy_equality,
        {
          Typing_warning.Sketchy_equality.result;
          left;
          right;
          left_trail;
          right_trail;
        } )
  else
    Typing_error_utils.add_typing_error
      ~env
      Typing_error.(
        primary
        @@ Primary.Trivial_strict_eq
             { pos; result; left; right; left_trail; right_trail })

let eq_incompatible_types env p ty1 ty2 =
  let tys1 = lazy (Typing_print.error env ty1)
  and tys2 = lazy (Typing_print.error env ty2) in
  Typing_error_utils.add_typing_error
    ~env
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

let rec assert_nontrivial p bop env ty1 ty2 ~as_warning =
  let ety_env = empty_expand_env in
  let (_, ty1) = Env.expand_type env ty1 in
  let (_, ety1, trail1) = TDef.force_expand_typedef ~ety_env env ty1 in
  let (_, ty2) = Env.expand_type env ty2 in
  let (_, ety2, trail2) = TDef.force_expand_typedef ~ety_env env ty2 in
  match (get_node ty1, get_node ty2) with
  (* Disallow `===` on distinct abstract enum types. *)
  (* Future: consider putting this in typed lint not type checking *)
  | (Tnewtype (e1, tyargs1, _), Tnewtype (e2, tyargs2, _))
    when Env.is_enum env e1 && Env.is_enum env e2 ->
    let (env, bound1) =
      Typing_utils.get_newtype_super env (get_reason ty1) e1 tyargs1
    in
    let (env, bound2) =
      Typing_utils.get_newtype_super env (get_reason ty2) e2 tyargs2
    in
    if is_arraykey bound1 && is_arraykey bound2 && not (String.equal e1 e2) then
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
        ~env
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
        ~env
        Typing_error.(
          wellformedness
          @@ Primary.Wellformedness.Void_usage
               { pos = p; reason = lazy (Reason.to_string "This is `void`" r) })
    | ((_, Tprim a), (_, Tnewtype (e, _, bound)))
      when Env.is_enum env e && bad_compare_prim_to_enum a bound ->
      trivial_comparison_error env p bop ty1 bound trail1 trail2 ~as_warning
    | ((_, Tnewtype (e, _, bound)), (_, Tprim a))
      when Env.is_enum env e && bad_compare_prim_to_enum a bound ->
      trivial_comparison_error env p bop bound ty2 trail1 trail2 ~as_warning
    | ((_, Tprim a), (_, Tprim b)) when not (Aast.equal_tprim a b) ->
      trivial_comparison_error env p bop ty1 ty2 trail1 trail2 ~as_warning
    | ((_, Toption ty1), (_, Tprim _)) ->
      assert_nontrivial p bop env ty1 ty2 ~as_warning
    | ((_, Tprim _), (_, Toption ty2)) ->
      assert_nontrivial p bop env ty1 ty2 ~as_warning
    | ((_, Toption ty1), (_, Toption ty2)) ->
      assert_nontrivial p bop env ty1 ty2 ~as_warning:true
    | ( ( _,
          ( Tany _ | Tnonnull | Tvec_or_dict _ | Tprim _ | Toption _ | Tdynamic
          | Tvar _ | Tfun _ | Tgeneric _ | Tnewtype _ | Tdependent _ | Tclass _
          | Ttuple _ | Tunion _ | Tintersection _ | Tshape _ | Taccess _
          | Tneg _ | Tlabel _ | Tclass_ptr _ ) ),
        _ ) ->
      ())

let assert_nontrivial = assert_nontrivial ~as_warning:false
