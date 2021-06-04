(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs
module Env = Tast_env
module MakeType = Typing_make_type
module Reason = Typing_reason

type validity =
  | Valid
  | Invalid : (Reason.decl_t * string) list -> validity

(* In hint positions, reified types are not resolved *)
type reification =
  | Resolved
  | Unresolved

type validation_state = {
  env: Env.env;
  ety_env: expand_env;
  validity: validity;
  like_context: bool;
  reification: reification;
  expanded_typedefs: SSet.t;
}

type error_emitter = Pos.t -> (Pos_or_decl.t * string) list -> unit

class virtual type_validator =
  object (this)
    inherit [validation_state] Type_visitor.decl_type_visitor

    method on_enum acc _ _ = acc

    method on_class acc _ _ tyl = List.fold_left tyl ~f:this#on_type ~init:acc

    method on_newtype acc _ _ tyl cstr ty =
      List.fold_left (ty :: cstr :: tyl) ~f:this#on_type ~init:acc

    method on_alias acc _ _ tyl ty =
      List.fold_left (ty :: tyl) ~f:this#on_type ~init:acc

    (* TODO(T88552052) is_concrete is a strange pattern here. The bool is used
     * to signal when a partially abstract type constant is directly accessed
     * in a way that would result in the value being used instead of
     * getting a constrained abstract type. This logic can be cleaned up by
     * eliminating partially abstract type constants and only using ttc_kind *)
    method on_typeconst acc _class _is_concrete typeconst =
      match typeconst.ttc_kind with
      | TCConcrete { tc_type } -> this#on_type acc tc_type
      | TCPartiallyAbstract { patc_constraint; patc_type } ->
        let acc = this#on_type acc patc_constraint in
        this#on_type acc patc_type
      | TCAbstract { atc_as_constraint; atc_super_constraint; atc_default } ->
        let acc = Option.fold ~f:this#on_type ~init:acc atc_as_constraint in
        let acc = Option.fold ~f:this#on_type ~init:acc atc_super_constraint in
        let acc = Option.fold ~f:this#on_type ~init:acc atc_default in
        acc

    method! on_taccess acc _r (root, id) =
      let (env, root) = Env.localize acc.env acc.ety_env root in
      let (env, tyl) = Env.get_concrete_supertypes env root in
      List.fold tyl ~init:acc ~f:(fun acc ty ->
          let (env, ty) = Env.expand_type env ty in
          match get_node ty with
          | Tclass ((_, class_name), _, _) ->
            let ( >>= ) = Option.( >>= ) in
            Option.value
              ~default:acc
              ( Env.get_class env class_name >>= fun class_ ->
                Decl_provider.Class.get_typeconst class_ (snd id)
                >>= fun typeconst ->
                let is_concrete =
                  match typeconst.ttc_kind with
                  | TCConcrete _ -> true
                  (* This handles the case for partially abstract type constants. In this case
                   * we know the assigned type will be chosen if the root is the same as the
                   * concrete supertype of the root.
                   *)
                  | TCPartiallyAbstract _ when phys_equal root ty -> true
                  | _ -> false
                in
                let ety_env = { acc.ety_env with this_ty = ty } in
                Some
                  (this#on_typeconst
                     { acc with ety_env }
                     class_
                     is_concrete
                     typeconst) )
          | _ -> acc)

    method! on_tapply acc r (pos, name) tyl =
      if Env.is_enum acc.env name && List.is_empty tyl then
        this#on_enum acc r (pos, name)
      else
        match Env.get_class_or_typedef acc.env name with
        | Some (Env.TypedefResult td) ->
          let {
            td_pos = _;
            td_vis = _;
            td_module = _;
            td_tparams;
            td_type;
            td_constraint;
          } =
            td
          in
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
            if Env.is_typedef_visible acc.env td then
              this#on_alias acc r (pos, name) tyl td_type
            else
              let td_constraint =
                match td_constraint with
                | None -> mk (r, Tmixed)
                | Some ty -> Decl_instantiate.instantiate subst ty
              in
              this#on_newtype acc r (pos, name) tyl td_constraint td_type
        | _ -> this#on_class acc r (pos, name) tyl

    (* Use_pos is the primary error position *)
    method validate_type
        env use_pos root_ty ?(reification = Unresolved) emit_error =
      let state =
        this#on_type
          {
            env;
            ety_env =
              {
                Typing_defs.empty_expand_env with
                this_ty =
                  Option.value
                    (Env.get_self_ty env)
                    ~default:(MakeType.nothing Reason.none);
              };
            expanded_typedefs = SSet.empty;
            validity = Valid;
            like_context = false;
            reification;
          }
          root_ty
      in
      match state.validity with
      | Invalid reasons ->
        emit_error
          use_pos
          (List.map reasons ~f:(fun (r, msg) -> (Reason.to_pos r, msg)))
      | Valid -> ()

    method validate_hint
        (env : Env.env)
        (hint : Aast.hint)
        ?(reification : reification = Unresolved)
        (emit_error : error_emitter) : unit =
      let hint_ty = Env.hint_to_ty env hint in
      this#validate_type env (fst hint) hint_ty ~reification emit_error

    (* Takes in and accumulates a list of reasons *)
    method invalid_list state reasons =
      match state.validity with
      | Valid -> { state with validity = Invalid reasons }
      | Invalid prev_reasons ->
        { state with validity = Invalid (prev_reasons @ reasons) }

    method invalid state r msg =
      match state.validity with
      | Valid -> { state with validity = Invalid [(r, msg)] }
      | Invalid _ -> state
  end
