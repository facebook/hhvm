(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Tast
open Ast_defs
open Typing_defs

module Env = Tast_env
module Reason = Typing_reason
module TySet = Typing_set
module Cls = Typing_classes_heap

type validity =
  | Valid
  | Invalid: Reason.t * string -> validity

type validation_state = {
  env: Env.env;
  validity: validity;
}

let update state new_validity =
  if state.validity = Valid
  then { state with validity = new_validity }
  else state

let visitor = object(this)
  inherit [validation_state] Type_visitor.type_visitor as super
  (* Only comes about because naming has reported an error and left Hany *)
  method! on_tany acc _ = acc
  (* Already reported an error *)
  method! on_terr acc _ = acc
  method! on_tprim acc r prim =
    match prim with
      | Aast.Tvoid -> update acc @@ Invalid (r, "the void type")
      | Aast.Tnoreturn -> update acc @@ Invalid (r, "the noreturn type")
      | _ -> acc
  method! on_tfun acc r _fun_type = update acc @@ Invalid (r, "a function type")
  method! on_tvar acc r _id = update acc @@ Invalid (r, "an unknown type")
  method! on_tabstract acc r ak _ty_opt =
    match ak with
    | AKenum _ -> acc
    | AKdependent (`this, _) -> acc
    | AKgeneric name when Env.is_fresh_generic_parameter name -> acc
    | AKgeneric name when AbstractKind.is_generic_dep_ty name ->
      let bounds = TySet.elements (Env.get_upper_bounds acc.env name) in
      List.fold_left bounds ~f:this#on_type ~init:acc
    | AKgeneric _ -> update acc @@
      Invalid (r, "a generic type parameter, because generics are erased at runtime")
    | AKnewtype _ -> update acc @@ Invalid (r, "a newtype")
    | AKdependent _ -> update acc @@ Invalid (r, "an expression dependent type")
  method! on_tanon acc r _arity _id =
    update acc @@ Invalid (r, "a function type")
  method! on_tunresolved acc r _tyl =
    update acc @@ Invalid (r, "a union")
  method! on_tobject acc r = update acc @@ Invalid (r, "the object type")
  method! on_tclass acc r cls exact tyl =
    match Env.get_class acc.env (snd cls) with
    | Some tc when Cls.kind tc = Ctrait ->
      update acc @@ Invalid (r, "a trait")
    | _ ->
      begin match tyl with
      | [] -> acc
      | tyl when List.for_all tyl this#is_wildcard -> acc
      | _ ->
        let acc = super#on_tclass acc r cls exact tyl in
        update acc @@ Invalid (r, "a type with generics, because generics are erased at runtime")
      end
  method! on_tapply acc r (_, name) tyl =
    if tyl <> [] && Typing_env.is_typedef name
    then update acc @@ Invalid (r, "a type with generics, because generics are erased at runtime")
    else acc
  method! on_tarraykind acc r _array_kind =
    update acc @@ Invalid (r, "an array type")
  method! on_taccess acc r taccess_type =
    match taccess_type with
    | (_, Tthis), _ -> update acc @@
      Invalid (r, "a late static bound type constant, because it can hide a generic")
    | _ -> acc
  method is_wildcard = function
    | _, Tabstract (AKgeneric name, _) ->
      Env.is_fresh_generic_parameter name
    | _ -> false
end

let validate_hint env hint op =
  let hint_ty = Env.hint_to_ty env hint in
  let should_suppress = ref false in
  let validate_type env ty =
    let state = visitor#on_type {env = env; validity = Valid} ty in
    match state.validity with
      | Invalid (r, msg) ->
        if not !should_suppress
        then Errors.invalid_is_as_expression_hint
          op (fst hint) (Reason.to_pos r) msg;
        should_suppress := true
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
