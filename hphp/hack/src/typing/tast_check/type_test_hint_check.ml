(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core
open Tast
open Typing_defs

module Env = Tast_env
module Reason = Typing_reason
module TySet = Typing_set

type validity =
  | Valid
  | Partial: locl ty -> validity
  | Invalid: 'a ty -> validity

type validation_state = {
  env: Env.env;
  validity: validity;
}

let update state new_validity = {
  (* Invalid > Partial > Valid *)
  state with validity =
    match state.validity, new_validity with
      | Valid, _
      | Partial _, Invalid _ -> new_validity
      | v, _ -> v;
}

let visitor = object(this)
  inherit [validation_state] Type_visitor.type_visitor as super
  method! on_tany acc r = update acc @@ Invalid (r, Tany)
  method! on_terr acc r = update acc @@ Invalid (r, Terr)
  method! on_tprim acc r prim =
    match prim with
      | Aast.Tvoid
      | Aast.Tnoreturn -> update acc @@ Invalid (r, Tprim prim)
      | _ -> acc
  method! on_tfun acc r fun_type = update acc @@ Invalid (r, Tfun fun_type)
  method! on_tvar acc r id = update acc @@ Invalid (r, Tvar id)
  method! on_tabstract acc r ak ty_opt =
    match ak with
    | AKenum _ -> acc
    | AKdependent (`this, _) -> acc
    | AKgeneric name when Env.is_fresh_generic_parameter name -> acc
    | AKgeneric name when AbstractKind.is_generic_dep_ty name ->
      let bounds = TySet.elements (Env.get_upper_bounds acc.env name) in
      List.fold_left bounds ~f:this#on_type ~init:acc
    | _ -> update acc @@ Invalid (r, Tabstract (ak, ty_opt))
  method! on_tanon acc r arity id =
    update acc @@ Invalid (r, Tanon (arity, id))
  method! on_tunresolved acc r tyl =
    update acc @@ Invalid (r, Tunresolved tyl)
  method! on_tobject acc r = update acc @@ Invalid (r, Tobject)
  method! on_tclass acc r cls tyl =
    match tyl with
      | [] -> acc
      | tyl when List.for_all tyl this#is_wildcard -> acc
      | _ ->
        let acc = super#on_tclass acc r cls tyl in
        begin match acc.validity with
          | Valid -> update acc @@ Partial (r, Tclass (cls, tyl))
          | _ -> acc
        end
  method! on_tapply acc r ((_, name) as id) tyl =
    if tyl <> [] && Typing_env.is_typedef name
    then update acc @@ Invalid (r, Tapply (id, tyl))
    else acc
  method! on_tarraykind acc r array_kind =
    update acc @@ Invalid (r, Tarraykind array_kind)
  method is_wildcard = function
    | _, Tabstract (AKgeneric name, _) ->
      Env.is_fresh_generic_parameter name
    | _ -> false
end

let print_type: type a. a ty_ -> string = function
  | Tclass (_, tyl) when tyl <> [] ->
    "a type with generics, because generics are erased at runtime"
  | Tapply (_, tyl) when tyl <> [] ->
    "a type with generics, because generics are erased at runtime"
  | ty_ -> Typing_print.error ty_

let validate_hint env hint op =
  let hint_ty = Env.hint_to_ty env hint in
  let should_suppress = ref false in
  let validate_type env ty =
    let state = visitor#on_type {env = env; validity = Valid} ty in
    match state.validity with
      | Invalid (r, ty_) ->
        if not !should_suppress
        then Errors.invalid_is_as_expression_hint
          op (fst hint) (Reason.to_pos r) (print_type ty_);
        should_suppress := true
      | Partial (r, ty_) ->
        if not !should_suppress
        then Errors.partially_valid_is_as_expression_hint
          op (fst hint) (Reason.to_pos r) (print_type ty_)
      | Valid -> ()
  in
  let env, hint_ty = Env.localize_with_dty_validator
    env hint_ty (validate_type env) in
  validate_type env hint_ty

let handler = object
  inherit Tast_visitor.handler_base
  method! at_expr env = function
    | _, Is (_, hint) -> validate_hint env hint "is"
    | _, As (_, hint, _) -> validate_hint env hint "as"
    | _ -> ()
end
