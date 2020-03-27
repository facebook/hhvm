(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
open Typing_defs
open Type_validator
module Env = Tast_env
module Reason = Typing_reason
module TySet = Typing_set
module Cls = Decl_provider.Class
module Nast = Aast

let validator =
  object (this)
    inherit type_validator as super

    (* Only comes about because naming has reported an error and left Hany *)
    method! on_tany acc _ = acc

    (* Already reported an error *)
    method! on_terr acc _ = acc

    method! on_tprim acc r prim =
      match prim with
      | Aast.Tvoid -> this#invalid acc r "the void type"
      | Aast.Tnoreturn -> this#invalid acc r "the noreturn type"
      | _ -> acc

    method! on_tfun acc r _fun_type = this#invalid acc r "a function type"

    method! on_tvar acc r _id = this#invalid acc r "an unknown type"

    method! on_typeconst acc is_concrete typeconst =
      match typeconst.ttc_abstract with
      | _ when snd typeconst.ttc_enforceable || is_concrete ->
        super#on_typeconst acc is_concrete typeconst
      | _ ->
        let (pos, tconst) = typeconst.ttc_name in
        let r = Reason.Rwitness pos in
        this#invalid acc r
        @@ "the abstract type constant "
        ^ tconst
        ^ " because it is not marked <<__Enforceable>>"

    method! on_tgeneric acc r name = this#check_generic acc r name

    method! on_newtype acc r _ _ _ _ =
      if acc.like_context then
        acc
      else
        this#invalid acc r "a newtype"

    method! on_tlike acc r ty =
      if TypecheckerOptions.like_casts (Tast_env.get_tcopt acc.env) then
        super#on_tlike { acc with like_context = true } r ty
      else
        this#invalid acc r "a like type"

    method! on_class acc r cls tyl =
      match Env.get_class acc.env (snd cls) with
      | Some tc ->
        let tparams = Cls.tparams tc in
        begin
          match tyl with
          | [] -> acc
          (* this case should really be handled by the fold2,
        but we still allow class hints without args in certain places *)
          | targs ->
            List.Or_unequal_lengths.(
              begin
                match
                  List.fold2 ~init:acc targs tparams ~f:(fun acc targ tparam ->
                      let covariant =
                        Ast_defs.(equal_variance tparam.tp_variance Covariant)
                      in
                      if this#is_wildcard targ then
                        acc
                      else if
                        Aast.(equal_reify_kind tparam.tp_reified Reified)
                        || (acc.like_context && covariant)
                      then
                        this#on_type acc targ
                      else
                        let error_message =
                          "a type with an erased generic type argument"
                        in
                        let error_message =
                          if
                            TypecheckerOptions.like_casts
                              (Tast_env.get_tcopt acc.env)
                          then
                            error_message
                            ^ ", except in a like cast when the corresponding type parameter is covariant"
                          else
                            error_message
                        in
                        this#invalid acc r error_message)
                with
                | Ok new_acc -> new_acc
                | Unequal_lengths -> acc (* arity error elsewhere *)
              end)
        end
      | None -> acc

    method! on_alias acc r id tyl ty =
      if List.is_empty tyl then
        this#on_type acc ty
      else if
        String.equal (snd id) Naming_special_names.FB.cIncorrectType
        && Int.equal (List.length tyl) 1
      then
        let ty = List.hd_exn tyl in
        this#on_type { acc with like_context = true } ty
      else if acc.like_context then
        super#on_alias acc r id tyl ty
      else
        this#invalid
          acc
          r
          "a type with generics, because generics are erased at runtime"

    method! on_tarray acc r tk_opt tv_opt =
      if acc.like_context then
        let acc = Option.fold ~f:this#on_type ~init:acc tk_opt in
        Option.fold ~f:this#on_type ~init:acc tv_opt
      else
        this#invalid acc r "an array type"

    method! on_tvarray_or_darray acc r tk_opt tv =
      if acc.like_context then
        let acc = Option.fold ~f:this#on_type ~init:acc tk_opt in
        this#on_type acc tv
      else
        this#invalid acc r "an array type"

    method is_wildcard ty =
      match get_node ty with
      | Tapply ((_, name), _) -> String.equal name SN.Typehints.wildcard
      | _ -> false

    method check_generic acc r name =
      match
        (Env.get_reified acc.env name, Env.get_enforceable acc.env name)
      with
      | (Nast.Erased, _) ->
        this#invalid acc r "an erased generic type parameter"
      | (Nast.SoftReified, _) ->
        this#invalid acc r "a soft reified generic type parameter"
      | (Nast.Reified, false) ->
        this#invalid
          acc
          r
          "a reified type parameter that is not marked <<__Enforceable>>"
      | (Nast.Reified, true) -> acc
  end

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env e =
      let validate hint op =
        validator#validate_hint
          env
          hint
          (Errors.invalid_is_as_expression_hint op)
      in
      match snd e with
      | Is (_, hint) -> validate hint "is"
      | As (_, hint, _) -> validate hint "as"
      | _ -> ()
  end
