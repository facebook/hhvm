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
  inherit [validation_state] Type_visitor.type_visitor as _super
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
      (* The when constraint guarnatees that there is a :: in the name *)
      let acc = match Str.split (Str.regexp_string "::") name with
      | [class_id; tconst_id] ->
        (* If a Taccess resolves to an abstract type constant, it will be given
         * to this visitor as a Tabstract, and the recursive bounds check below
         * will eventually resolve to the abstract type constant's constraint.
         * However, a subtype of this constraint could have a type parameter, so
         * we check whether the abstract type constant is enforceable. In the case
         * where Taccess is concrete, the locl ty will have resolved to a
         * Tclass/Tprim/etc, so it won't be checked by this method *)
        let open Option in
        let tconst_opt = Env.get_class acc.env class_id >>=
          (fun cls -> Cls.get_typeconst cls tconst_id) in
        Option.value_map ~default:acc tconst_opt ~f:(fun tconst ->
          if not (snd tconst.ttc_enforceable)
          then update acc @@
            Invalid (r, "the abstract type constant " ^
              tconst_id ^ " because it is not marked <<__Enforceable>>")
          else acc
        )
      | _ ->
        acc in


      let bounds = TySet.elements (Env.get_upper_bounds acc.env name) in
      List.fold_left bounds ~f:this#on_type ~init:acc
    | AKgeneric name ->
      begin match Env.get_reified acc.env name, Env.get_enforceable acc.env name with
      | Nast.Erased, _ -> update acc @@
        Invalid (r, "an erased generic type parameter")
      | Nast.SoftReified, _ -> update acc @@
        Invalid (r, "a soft reified generic type parameter")
      | Nast.Reified, false -> update acc @@
        Invalid (r, "a reified type parameter that is not marked <<__Enforceable>>")
      | Nast.Reified, true ->
        acc end
    | AKnewtype _ -> update acc @@ Invalid (r, "a newtype")
    | AKdependent _ -> update acc @@ Invalid (r, "an expression dependent type")
  method! on_tanon acc r _arity _id =
    update acc @@ Invalid (r, "a function type")
  method! on_tunresolved acc r _tyl =
    update acc @@ Invalid (r, "a union")
  method! on_tobject acc r = update acc @@ Invalid (r, "the object type")
  method! on_tclass acc r cls _ tyl =
    match Env.get_class acc.env (snd cls) with
    | Some tc ->
      let tparams = Cls.tparams tc in
      begin match tyl with
      | [] -> acc (* this case should really be handled by the fold2,
        but we still allow class hints without args in certain places *)
      | tyl ->
        let open List.Or_unequal_lengths in
        begin match List.fold2 ~init:acc tyl tparams ~f:(fun acc targ tparam ->
          if this#is_wildcard targ
          then acc
          else
            match tparam.tp_reified with
            | Nast.Erased -> update acc @@ Invalid (r, "a type with an erased generic type argument")
            | Nast.SoftReified -> update acc @@ Invalid (r, "a type with a soft reified type argument")
            | Nast.Reified -> this#on_type acc targ
        ) with
        | Ok new_acc -> new_acc
        | Unequal_lengths -> acc (* arity error elsewhere *)
        end
      end
    | None -> acc
  method! on_tapply acc r (_, name) tyl =
    if tyl <> [] && Typing_env.is_typedef name
    then update acc @@ Invalid (r, "a type with generics, because generics are erased at runtime")
    else acc
  method! on_tarraykind acc r _array_kind =
    update acc @@ Invalid (r, "an array type")
  method is_wildcard = function
    | _, Tabstract (AKgeneric name, _) ->
      Env.is_fresh_generic_parameter name
    | _ -> false
end

let validate_type env root_ty emit_error =
  let should_suppress = ref false in
  let validate env ty =
    let state = visitor#on_type {env = env; validity = Valid} ty in
    match state.validity with
      | Invalid (r, msg) ->
        if not !should_suppress
        then emit_error (Reason.to_pos (fst root_ty)) (Reason.to_pos r) msg;
        should_suppress := true
      | Valid -> ()
  in
  let env, root_ty =
    Env.localize_with_dty_validator env root_ty (validate env) in
  validate env root_ty

let validate_hint env hint emit_error =
  let hint_ty = Env.hint_to_ty env hint in
  validate_type env hint_ty emit_error

let handler = object
  inherit Tast_visitor.handler_base
  method! at_expr env = function
    | _, Is (_, hint) -> validate_hint env hint (Errors.invalid_is_as_expression_hint "is")
    | _, As (_, hint, _) -> validate_hint env hint (Errors.invalid_is_as_expression_hint "as")
    | _ -> ()
end
