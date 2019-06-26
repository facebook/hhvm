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
module Cls = Decl_provider.Class

type validity =
  | Valid
  | Invalid: Reason.t * string -> validity

type validation_state = {
  env: Env.env;
  ety_env: expand_env;
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

  method! on_taccess acc r (root, ids) =
    (* We care only about the last type constant we access in the chain
     * this::T1::T2::Tn. So we reverse the ids to get the last one then we resolve
     * up to that point using localize to determine the root. i.e. we resolve
     *   root = (this::T1::T2)
     *   id = Tn
     *)
    match List.rev ids with
    | [] ->
      this#on_type acc root
    | (_, tconst)::rest ->
      let root = if rest = [] then root else (r, Taccess (root, List.rev rest)) in
      let env, root = Env.localize acc.env acc.ety_env root in
      let env, tyl = Env.get_concrete_supertypes env root in
      List.fold tyl ~init:acc ~f:begin fun acc ty ->
        match snd ty with
        | Typing_defs.Tclass ((_, class_name), _, _) ->
          let (>>=) = Option.(>>=) in
          Option.value ~default:acc begin
            Env.get_class env class_name >>= fun class_ ->
            Cls.get_typeconst class_ tconst >>= fun typeconst ->
            match typeconst.ttc_abstract with
            | _ when (snd typeconst.ttc_enforceable) -> Some acc
            | TCConcrete -> Some acc
            (* This handles the case for partially abstract type constants. In this case
             * we know the assigned type will be chosen if the root is the same as the
             * concrete supertype of the root.
             *)
            | TCPartiallyAbstract when phys_equal root ty ->
              Some acc
            | _ ->
              Some (update acc @@
                Invalid (Reason.Rwitness (fst typeconst.ttc_name), "the abstract type constant " ^
                  tconst ^ " because it is not marked <<__Enforceable>>")
              )
          end
        | _ -> acc
      end

  method! on_tabstract acc r ak _ty_opt =
    match ak with
    | AKenum _ -> acc
    | AKdependent (`this) -> acc
    | AKgeneric name when Env.is_fresh_generic_parameter name  ||
                          AbstractKind.is_generic_dep_ty name -> acc
    | AKgeneric name ->
      this#check_generic acc r name
    | AKnewtype _ -> update acc @@ Invalid (r, "a newtype")
    | AKdependent _ -> update acc @@ Invalid (r, "an expression dependent type")
  method! on_tanon acc r _arity _id =
    update acc @@ Invalid (r, "a function type")
  method! on_tunion acc r _tyl =
    update acc @@ Invalid (r, "a union")
  method! on_tobject acc r = update acc @@ Invalid (r, "the object type")
  method! on_tclass acc r cls _ tyl =
    match Env.get_class acc.env (snd cls) with
    | Some tc ->
      let tparams = Cls.tparams tc in
      begin match tyl with
      | [] -> acc (* this case should really be handled by the fold2,
        but we still allow class hints without args in certain places *)
      | targs ->
        let open List.Or_unequal_lengths in
        begin match List.fold2 ~init:acc targs tparams ~f:(fun acc targ tparam ->
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

  method is_wildcard: type a. a ty -> bool = function
    | _, Tabstract (AKgeneric name, _) ->
      Env.is_fresh_generic_parameter name
    | _ -> false
  method check_generic acc r name =
    match Env.get_reified acc.env name, Env.get_enforceable acc.env name with
    | Nast.Erased, _ -> update acc @@
      Invalid (r, "an erased generic type parameter")
    | Nast.SoftReified, _ -> update acc @@
      Invalid (r, "a soft reified generic type parameter")
    | Nast.Reified, false -> update acc @@
      Invalid (r, "a reified type parameter that is not marked <<__Enforceable>>")
    | Nast.Reified, true ->
      acc
  method check_class_targs: type a. _ -> _ -> _ -> a ty list -> _ = fun acc r c_name targs ->
    match Env.get_class acc.env c_name with
    | Some tc ->
      let tparams = Cls.tparams tc in
      begin match targs with
      | [] -> acc (* this case should really be handled by the fold2,
        but we still allow class hints without args in certain places *)
      | targs ->
        let open List.Or_unequal_lengths in
        begin match List.fold2 ~init:acc targs tparams ~f:(fun acc targ tparam ->
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
end

let validate_type env root_ty emit_error =
  let should_suppress = ref false in
  let validate env ety_env ty =
    let state = visitor#on_type {env; ety_env; validity = Valid} ty in
    match state.validity with
      | Invalid (r, msg) ->
        if not !should_suppress
        then emit_error (Reason.to_pos (fst root_ty)) (Reason.to_pos r) msg;
        should_suppress := true
      | Valid -> ()
  in
  let env, root_ty =
    Env.localize_with_dty_validator env root_ty (validate env) in
  validate env {
    type_expansions = [];
    substs = SMap.empty;
    this_ty = Option.value ~default:(Reason.none, Tany) @@ Env.get_self env;
    from_class = None;
    validate_dty = None;
  } root_ty

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
