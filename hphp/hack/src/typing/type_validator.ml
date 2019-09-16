(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Typing_defs
module Env = Tast_env
module MakeType = Typing_make_type
module Reason = Typing_reason

type validity =
  | Valid
  | Invalid : Reason.t * string -> validity

type validation_state = {
  env: Env.env;
  ety_env: expand_env;
  validity: validity;
  like_context: bool;
  expanded_typedefs: SSet.t;
}

class virtual type_validator =
  object (this)
    inherit [validation_state] Type_visitor.decl_type_visitor

    method on_enum acc _ _ = acc

    method on_class acc _ _ tyl = List.fold_left tyl ~f:this#on_type ~init:acc

    method on_newtype acc _ _ tyl cstr ty =
      List.fold_left (ty :: cstr :: tyl) ~f:this#on_type ~init:acc

    method on_alias acc _ _ tyl ty =
      List.fold_left (ty :: tyl) ~f:this#on_type ~init:acc

    method on_typeconst acc _ typeconst =
      let acc =
        Option.fold ~f:this#on_type ~init:acc typeconst.ttc_constraint
      in
      let acc = Option.fold ~f:this#on_type ~init:acc typeconst.ttc_type in
      acc

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
                Decl_provider.Class.get_typeconst class_ tconst
                >>= fun typeconst ->
                let is_concrete =
                  match typeconst.ttc_abstract with
                  | TCConcrete -> true
                  (* This handles the case for partially abstract type constants. In this case
                   * we know the assigned type will be chosen if the root is the same as the
                   * concrete supertype of the root.
                   *)
                  | TCPartiallyAbstract when phys_equal root ty -> true
                  | _ -> false
                in
                let ety_env = { acc.ety_env with this_ty = ty } in
                Some
                  (this#on_typeconst { acc with ety_env } is_concrete typeconst)
                )
            | _ -> acc)

    method! on_tapply acc r (pos, name) tyl =
      if Env.is_enum acc.env name && tyl = [] then
        this#on_enum acc r (pos, name)
      else
        match Env.get_typedef acc.env name with
        | None -> this#on_class acc r (pos, name) tyl
        | Some
            {
              td_pos;
              td_vis;
              td_tparams;
              td_type;
              td_constraint;
              td_decl_errors = _;
            } ->
          if SSet.mem name acc.expanded_typedefs then
            acc
          else
            let acc =
              {
                acc with
                expanded_typedefs = SSet.add name acc.expanded_typedefs;
              }
            in
            let subst = Decl_instantiate.make_subst td_tparams tyl in
            let td_type = Decl_instantiate.instantiate subst td_type in
            (match td_vis with
            | Aast.Opaque when Pos.filename td_pos <> Env.get_file acc.env ->
              let td_constraint =
                match td_constraint with
                | None -> (r, Tmixed)
                | Some ty -> Decl_instantiate.instantiate subst ty
              in
              this#on_newtype acc r (pos, name) tyl td_constraint td_type
            | _ -> this#on_alias acc r (pos, name) tyl td_type)

    method validate_type env root_ty emit_error =
      let should_suppress = ref false in
      let validate env ty =
        let state =
          this#on_type
            {
              env;
              ety_env =
                {
                  type_expansions = [];
                  substs = SMap.empty;
                  this_ty =
                    Option.value
                      (Env.get_self env)
                      ~default:(MakeType.nothing Reason.none);
                  from_class = Some Aast.CIstatic;
                };
              expanded_typedefs = SSet.empty;
              validity = Valid;
              like_context = false;
            }
            ty
        in
        match state.validity with
        | Invalid (r, msg) ->
          if not !should_suppress then
            emit_error (Reason.to_pos (fst root_ty)) (Reason.to_pos r) msg;
          should_suppress := true
        | Valid -> ()
      in
      validate env root_ty

    method validate_hint env hint emit_error =
      let hint_ty = Env.hint_to_ty env hint in
      this#validate_type env hint_ty emit_error

    method invalid state r msg =
      if state.validity = Valid then
        { state with validity = Invalid (r, msg) }
      else
        state
  end
