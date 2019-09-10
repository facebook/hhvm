(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
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

    method! on_taccess acc r (root, ids) =
      (* We care only about the last type constant we access in the chain
       * this::T1::T2::Tn. So we reverse the ids to get the last one then we resolve
       * up to that point using localize to determine the root. i.e. we resolve
       *   root = (this::T1::T2)
       *   id = Tn
       *)
      match List.rev ids with
      | [] -> this#on_type acc root
      | (_, tconst) :: rest ->
        let root =
          if rest = [] then
            root
          else
            (r, Taccess (root, List.rev rest))
        in
        let (env, root) = Env.localize acc.env acc.ety_env root in
        let (env, tyl) = Env.get_concrete_supertypes env root in
        List.fold tyl ~init:acc ~f:(fun acc ty ->
            match snd ty with
            | Typing_defs.Tclass ((_, class_name), _, _) ->
              let ( >>= ) = Option.( >>= ) in
              Option.value
                ~default:acc
                ( Env.get_class env class_name
                >>= fun class_ ->
                Cls.get_typeconst class_ tconst
                >>= fun typeconst ->
                match typeconst.ttc_abstract with
                | _ when snd typeconst.ttc_enforceable -> Some acc
                | TCConcrete -> Some acc
                (* This handles the case for partially abstract type constants. In this case
                 * we know the assigned type will be chosen if the root is the same as the
                 * concrete supertype of the root.
                 *)
                | TCPartiallyAbstract when phys_equal root ty -> Some acc
                | _ ->
                  let r = Reason.Rwitness (fst typeconst.ttc_name) in
                  let acc =
                    this#invalid acc r
                    @@ "the abstract type constant "
                    ^ tconst
                    ^ " because it is not marked <<__Enforceable>>"
                  in
                  Some acc )
            | _ -> acc)

    method! on_tabstract acc r ak _ty_opt =
      match ak with
      | AKnewtype (cid, _) when Env.is_enum acc.env cid -> acc
      | AKdependent DTthis -> acc
      | AKgeneric name
        when Env.is_fresh_generic_parameter name
             || AbstractKind.is_generic_dep_ty name ->
        acc
      | AKgeneric name -> this#check_generic acc r name
      | AKnewtype _ -> this#invalid acc r "a newtype"
      | AKdependent _ -> this#invalid acc r "an expression dependent type"

    method! on_tanon acc r _arity _id = this#invalid acc r "a function type"

    method! on_tunion acc r tyl =
      if
        TypecheckerOptions.like_types (Tast_env.get_tcopt acc.env)
        && Typing_utils.is_dynamic
             (Env.tast_env_as_typing_env acc.env)
             (r, Tunion tyl)
      then
        super#on_tunion { acc with like_context = true } r tyl
      else
        this#invalid acc r "a union"

    method! on_tobject acc r = this#invalid acc r "the object type"

    method! on_tclass acc r cls _ tyl =
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
                        tparam.tp_variance = Ast_defs.Covariant
                      in
                      let like_types_enabled =
                        TypecheckerOptions.like_types
                          (Tast_env.get_tcopt acc.env)
                      in
                      if this#is_wildcard targ then
                        acc
                      else if
                        tparam.tp_reified = Nast.Reified
                        || (acc.like_context && covariant && like_types_enabled)
                      then
                        this#on_type acc targ
                      else
                        let error_message =
                          "a type with an erased generic type argument"
                        in
                        let error_message =
                          if like_types_enabled then
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

    method! on_tapply acc r (_, name) tyl =
      if tyl <> [] && Typing_env.is_typedef name then
        this#invalid
          acc
          r
          "a type with generics, because generics are erased at runtime"
      else
        acc

    method! on_tarraykind acc r _array_kind =
      this#invalid acc r "an array type"

    method is_wildcard : type a. a ty -> bool =
      function
      | (_, Tabstract (AKgeneric name, _)) ->
        Env.is_fresh_generic_parameter name
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
