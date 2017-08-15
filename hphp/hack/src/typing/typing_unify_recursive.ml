(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core
open Typing_defs

module Env = Typing_env
module TUtils = Typing_utils


(* How does type variable n occur in type rty?
 *   DoesOccurUnderOptions                e.g. ??n
 *   DoesOccurUnderUnresolvedOptions      e.g. int | string | ?n
 *   DoesOccurUnderUnresolved             e.g. int | string | n
 *   DoesOccurAtTop                       e.g. n
 *   DoesOccur                            e.g. array<n>
 *   DoesNotOccur                         e.g. array<int>
 *)
type occursResult =
  | DoesOccurUnderOptions
  | DoesOccurUnderUnresolvedOptions of locl ty list
  | DoesOccurUnderUnresolved of locl ty list
  | DoesOccur
  | DoesNotOccur
  | DoesOccurAtTop

(* This is a simple occurs-check for types and various other components
 * of types. Return trues if variable [n] occurs anywhere in [rty]
 *)
let rec occurs env n rty =
  let _, ty = rty in
  match ty with
  | Tvar n' ->
    let _, n'' = Env.get_var env n' in
    begin
      n == n'' || (match IMap.get n'' env.Env.tenv with
      | Some ty -> occurs env n ty
      | None -> false)
    end
  | Terr | Tany | Tmixed | Tanon _ | Tprim _ | Tobject -> false
  | Toption t -> occurs env n t
  | Ttuple ts | Tunresolved ts | Tclass(_,ts) -> occurs_list env n ts
  | Tabstract(ak,topt) -> occurs_ak env n ak || occurs_opt  env n topt
  | Tarraykind ak -> occurs_array env n ak
  | Tfun ft -> occurs_ft env n ft
  | Tshape(_,sm) ->
      Nast.ShapeMap.exists (fun _ { sft_ty; _ } -> occurs env n sft_ty) sm
and occurs_opt env n topt =
  match topt with
  | None -> false
  | Some t -> occurs env n t
and occurs_list env n ts =
  match ts with
  | [] -> false
  | t::ts -> occurs  env n t || occurs_list env n ts
and occurs_ak env n ak =
  match ak with
  | AKnewtype(_, ts) -> occurs_list env n ts
  | AKenum _ -> false
  | AKgeneric _ -> false
  | AKdependent _ -> false
and occurs_array env n ak =
  match ak with
  | AKany -> false
  | AKvarray_or_darray t
  | AKvarray t
  | AKvec t -> occurs env n t
  | AKdarray (t1, t2)
  | AKmap(t1,t2) -> occurs env n t1 || occurs env n t2
  | AKempty -> false
  | AKshape sm -> Nast.ShapeMap.exists
      (fun _ (t1,t2) -> occurs env n t1 || occurs env n t2) sm
  | AKtuple m -> IMap.exists (fun _ t -> occurs env n t) m
and occurs_ft env n ft =
    occurs_params env n ft.ft_params || occurs env n ft.ft_ret
and occurs_params env n p =
  List.exists p (fun (_,t) -> occurs env n t)

(* Does variable [n] occur at top-level in [ty] or under any number
 * of Toption wrappers, eliding singleton Tunresolved?
 * Return Some k if it occurs under k levels of Toption,
 * or None otherwise.
 *)
let rec occursUnderOptions level env n ty =
  match snd ty with
  | Tvar n' ->
    let _, n'' = Env.get_var env n' in
    begin
      if n == n''
      then Some level
      else
      begin match IMap.get n'' env.Env.tenv with
      | Some t -> occursUnderOptions level env n t
      | None -> None
      end
    end
  | Toption t -> occursUnderOptions (level+1) env n t
  | Tunresolved [t] -> occursUnderOptions level env n t
  | _ -> None

(* Given a list of types [tyl], locate the first type that is
 * the variable [n] wrapped under k (possibly zero) levels of
 * Toption wrappers. Return Some(k, tys) where tys is the
 * other types in tyl (in arbitrary order). Otherwise return None.
*)
let rec findFirstVarOrOptionVar env n tyl =
  match tyl with
  | [] ->
    None

  | ty::tyl ->
    match occursUnderOptions 0 env n ty with
    | Some count ->
      begin match findFirstVarOrOptionVar env n tyl with
      | None -> Some(count,tyl)
      | Some _ -> None
      end
    | None ->
      begin match findFirstVarOrOptionVar env n tyl with
      | None -> None
      | Some (count,tys) -> Some (count,ty::tys)
      end

(* Does variable n (which must be normalized with respect to env.subst)
 * occur inside type rty? See comment above occursResult for meaning
 * of the result.
 *)
let occursTop env n rty =
  match occursUnderOptions 0 env n rty with
  | Some 0 -> DoesOccurAtTop
  | Some _ -> DoesOccurUnderOptions
  | None ->
    let _, (_, ty) = Env.expand_type env rty in
    match ty with
    | Tunresolved ts ->
      begin match findFirstVarOrOptionVar env n ts with
      | None -> if occurs_list env n ts then DoesOccur else DoesNotOccur
      | Some (0, ts) ->
        if occurs_list env n ts then DoesOccur
        else DoesOccurUnderUnresolved ts
      | Some (_, ts) ->
        if occurs_list env n ts then DoesOccur
        else DoesOccurUnderUnresolvedOptions ts
      end
    | _ -> if occurs env n rty then DoesOccur else DoesNotOccur


(* Wrapped version of Env.add that deals with the possibility of
 * the variable [x] occurring in type [ty], taking three possible
 * courses of action:
 *   1. If [x] does not occur in [ty] then the assignment x:=ty is
        added to the substitution by calling Env.add
 *   2. If [x] does occur in [ty] but it is possible to compute
        a substitution that unifies [x] with [ty] then do so.
 *   3. Otherwise log an "circular type" error
 * Note that as a consequence, cycles in types are never created through
 * unification and so there is no need to maintain lists of "seen"
 * variables in functions that recurse over type structure.
 *)
let add env x ty =
  let env, x' = Env.get_var env x in
  match occursTop env x' ty with
  | DoesOccurAtTop ->
    env

  | DoesNotOccur ->
    Env.add env x ty

  | DoesOccur ->
    begin
      Errors.unification_cycle
        (Reason.to_pos (fst ty)) (Typing_print.full_rec env x' ty);
      Env.add env x (fst ty, Terr)
    end

    (* We solve the unification problem [n] against ?+ [n] by
     * the substitution n := ?m for fresh variable m
     *)
  | DoesOccurUnderOptions ->
    let env, ty' = Env.fresh_unresolved_type env in
    Env.add env x (fst ty, Toption ty')

    (* We solve the unification problem [n]
     * against (?+ [n] | t1 | ... |k) by the
     * substitution n := ?(m | t1 | ... | tk) for fresh variable m
     *)
  | DoesOccurUnderUnresolvedOptions ts ->
    let env, ty' = Env.fresh_unresolved_type env in
    Env.add env x (fst ty, Toption (fst ty, Tunresolved (ty'::ts)))

    (* We solve the unification problem [n]
     * against ([n] | t1 | ... |k) by the
     * substitution n := (m | t1 | ... | tk) for fresh variable m
     *)
  | DoesOccurUnderUnresolved ts ->
    let env, ty' = Env.fresh_unresolved_type env in
    Env.add env x (fst ty, Tunresolved (ty'::ts))
